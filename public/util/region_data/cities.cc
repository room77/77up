// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

#include <algorithm>
#include <cmath>
#include <functional>

#include "util/region_data/cities.h"

#include "util/init/init.h"
#include "util/string/strutil.h"
#include "util/templates/functional.h"
#include "util/geo/latlong.h"
#include "util/region_data/countries.h"
#include "util/region_data/states.h"
#include "util/region_data/utils.h"
#include "util/time/simple_timer.h"

FLAG_string(em_cities_id, "em_cities",
            "The default entity manager id for cities");

FLAG_bool(region_cities_use_scored_cities, false,
          "Loads only the scored cities file.");

FLAG_string(region_cities_base_file,
            "/home/share/data/region_data/cities/current/cities.csv",
            "List of cities.");
FLAG_string(region_cities_scored_file,
            "/home/share/data/suggest/locations/stats/current/city/cities_scored.csv",
            "List of cities.");

FLAG_bool(region_cities_create_latlong_index, false,
          "Set to true if lat long index should be created. "
          "Set by default if --region_cities_use_scored_cities is specified.");

FLAG_int(region_cities_dedup_min_distance, 23,
         "The minimum distance (in km) between two cities for them to be "
         "considered different.");
FLAG_int(region_cities_dedup_min_score, 5,
         "The minimum number of hotels a city added at runtime must have for "
         "it to be considered safe from dedup.");
FLAG_int(region_cities_dedup_min_name_edit_distance, 3,
         "The minimum edit distance between two city names for them to be "
         "considered different.");
FLAG_int(region_cities_max_items_per_key, -1,
         "The maximum number of items stored per key during indexing.");

namespace region_data {

tCityReply city_storage_to_reply::operator() (const tCity& city) const {
  tCityReply reply = super::operator()(city);

  // State Info.
  if (!city.state_iso_code_short.empty()) {
    reply.state_code = city.state_iso_code_short;
    const tState* s = States::Instance().LookupByCode(
        city.state_iso_code_short, city.country_iso_code_2char);
    if (s != nullptr) reply.state_name = s->name;
  }

  // Country info.
  if (!city.country_iso_code_2char.empty()) {
    reply.country_code = city.country_iso_code_2char;
    const tCountry* ctry = Countries::Instance().LookupByCode(
        city.country_iso_code_2char);
    if (ctry != nullptr) reply.country_name = ctry->name;
  }

  return reply;
}

bool Cities::Initialize() {
  string csv_file;
  if (gFlag_region_cities_use_scored_cities) {
    gFlag_region_cities_create_latlong_index = true;
    csv_file = gFlag_region_cities_scored_file;
  } else {
    csv_file = gFlag_region_cities_base_file;
  }

  super::ConfigParams params;
  params.file = csv_file;
  params.create_entity_ids = true;
  params.build_entity_id_index = true;
  params.build_name_index = true;
  params.name_index_is_unique = false;
  params.build_latlong_index = gFlag_region_cities_create_latlong_index;
  params.name_index_max_items_per_key = gFlag_region_cities_max_items_per_key;

  LOG(INFO) << "Reading cities.";
  ::util::time::SimpleTimer timer;
  timer.Start();

  ASSERT(super::Initialize(params));
  // Build more indices for the cities.
  BuildIndices();

  timer.Stop();
  LOG(INFO) << "Finished reading " << storage_.size() << " cities. Took "
         << timer.GetDurationSec() << " seconds.";

  return true;
}

void Cities::BuildIndices() {
  // Sort the collection to have the cities with the highest hotels at the top.
  // TODO(pramodg): The csv files should be already sorted correctly.
  // See if there is really a need for sorting here.
  // sort(storage_.begin(), storage_.end(), order_cities_by_score());

  // Index by city name, basic indices etc.
  for (const shared_ptr<tCity>& city : storage_)
    PrimaryIndicesForCity(city.get());
}

void Cities::PrimaryIndicesForCity(const tCity* city) {
  if (!city) return;

  const string city_name = utils::NormalizeString(city->name);
  if (city_name.empty()) return;

  const string state_code = utils::NormalizeString(city->state_iso_code_short);
  const string country_code = utils::NormalizeString(city->country_iso_code_2char);

  // Index the city by different primary keywords.

  // Index the city (uniquely) by metro code.
  if (city->metro_code.size())
    city_metro_code_index_.AddToIndex(utils::NormalizeString(city->metro_code), city, true);

  // Index by state.
  string state_name;
  if (!state_code.empty()) {
    IndexCityByKey(city, city_name + " " + state_code);

    state_name = utils::NormalizeString(States::Instance().GetStateName(state_code, country_code));
    if (!state_name.empty()) IndexCityByKey(city, city_name + " " + state_name);
  }

  // Index by country.
  string country_name;
  if (!country_code.empty()) {
    IndexCityByKey(city, city_name + " " + country_code);

    country_name = utils::NormalizeString(Countries::Instance().GetCountryName(country_code));
    if (!country_name.empty()) IndexCityByKey(city, city_name + " " + country_name);

    // TODO(pramodg): Use alternate country names instead.
    if (country_code == "us") IndexCityByKey(city, city_name + " usa");
  }

  // Index by state and country.
  if (!state_code.empty() &&  !country_code.empty()) {
    IndexCityByKey(city, city_name + " " + state_code + " " + country_code);

    if (!state_name.empty()) {
      IndexCityByKey(city, city_name + " " + state_name + " " + country_code);

      if (!country_name.empty()) {
        IndexCityByKey(city, city_name + " " + state_code + " " + country_name);
        IndexCityByKey(city, city_name + " " + state_name + " " + country_name);

        // TODO(pramodg): Use alternate state/country names instead.
        if (country_code == "us") {
          IndexCityByKey(city, city_name + " " + state_code + " usa");
          IndexCityByKey(city, city_name + " " + state_name + " usa");
        }
      }
    }
  }
}

void Cities::IndexCityByKey(const tCity* city, const string& key) {
  if (key.empty()) return;

  // Add the key to the name index.
  name_index_.AddToIndex(key, city, false);
}

const tCity* Cities::LookupByCode(const string& code) const {
  return city_metro_code_index_.RetrieveUnique(utils::NormalizeString(code));
}

const tCity* Cities::LookupByNameAndCode(const string& input_name,
    const string& state_code, const string& country_code) const {
  vector<const tCity*> result;
  // Try city_name + state_code + country_code.
  if (LookupByName(input_name + " " + state_code + " " + country_code, &result, 1))
    return result[0];

  // We failed, lets try for partial matches.
  // If there was already a state code, and there was a country code, remove the state code and
  // try city_name + country_code.
  if (state_code.size() && country_code.size()) {
    result.clear();
    if (LookupByName(input_name + " " + country_code, &result, 1))
      return result[0];
  }

  // We could not find anything. We do not do a lookup just by name as it is not unique enough.
  // If this fails, the caller should call LookupByName directly and do a filter as necessary.
  // NOTE: DO NOT add that code here. A lot of people expect this call to return a very unique key.
  // At some point we may even want to remove the lookup by just [city_name state_code].
  return nullptr;
}

vector<const tCity*> Cities::NormalizeAndLookup(const string& city_str,
    const string& state_str, const string& country_str) const {
  vector<const tCity*> ret;
  const tCountry* country = Countries::Instance().LookupByCodeMatchAny(country_str);
  if (!country) country = Countries::Instance().LookupUniqueByName(country_str);

  const tState* state = nullptr;
  if (country) {
    // If we have country, let's try to get the state as well.
    state = States::Instance().LookupByCode(state_str, country->iso_code_2char);
    if (!state)
      state = States::Instance().LookupByNameAndCountryCode(state_str, country->iso_code_2char);

    // If we also have state, attempt to lookup the city.
    if (state) {
      const tCity* cp = Cities::Instance().LookupByNameAndCode(
          city_str, state->iso_code_short, country->iso_code_2char);
      if (cp) ret.push_back(cp);
    }
  }

  if (ret.empty()) {
    // We could not match the state, return all matching cities within the
    // country and let the user decide which one to use (e.g. using lat/lon).
    vector<const tCity*> matches;
    Cities::Instance().LookupByName(city_str, &matches);
    country_code_matcher cc_matcher;
    for (const tCity* match : matches) {
      // If we have state and states don't match
      if (state && !match->state_iso_code_short.empty() && !state->iso_code_short.empty() &&
          match->state_iso_code_short != state->iso_code_short) continue;

      // Stay on the safe side. If country is missing, don't match.
      if (country && !cc_matcher(match->country_iso_code_2char, country->iso_code_2char)) continue;

      // If state / country checks are fine, add the city to the results.
      // Note that end-user may still need to do some filtering.
      ret.push_back(match);
    }
  }

  return ret;
}

bool Cities::GetCityStateCountryInfo(const tCity* city,
                                     tCityStateCountry *result) {
  if (!city) return false;

  // Get the country.
  const tCountry *country =
      Countries::Instance().LookupByCode(city->country_iso_code_2char);
  if (!country) return false;

  // Check if the state is valid.
  const tState* state = nullptr;
  if (!city->state_iso_code_short.empty()) {
    state = States::Instance().LookupByCode(city->state_iso_code_short,
                                             city->country_iso_code_2char);
  }

  result->city_code = city->metro_code;
  result->city_name = city->name;
  result->state_code = city->state_iso_code_short;
  result->state_name = state != nullptr ? state->name : "";
  result->country_code = city->country_iso_code_2char;
  result->country_name = country->name;
  result->latitude = city->lat;
  result->longitude = city->lon;
  result->MakeCommonName();
  return true;
}

void Cities::Dedup(vector<const tCity*>* cities) const {
  super::Dedup(cities);

  // Map from country -> vector of cities. Not all cities have state info.
  // Thus we do not take it into account while considering duplicate cities.
  unordered_map<string, vector<const tCity*> > partition_by_country;
  for (const tCity* city : *cities)
    partition_by_country[city->country_iso_code_2char].push_back(city);

  unordered_set<const tCity*> duplicates;
  for (auto iter = partition_by_country.begin();
      iter != partition_by_country.end(); ++iter) {
    const vector<const tCity*>& cities_in_same_country = iter->second;

    for (int i = cities_in_same_country.size() - 1; i >= 0; --i) {
      const tCity* candidate = cities_in_same_country[i];

      // If there are significantly many hotels in the city, consider it to be
      // valid.
      if (candidate->score > gFlag_region_cities_dedup_min_score)
        continue;

      // We cannot evaluate without lat long.
      if (!candidate->has_lat_long()) continue;

      LatLong candidate_latlong = LatLong::Create(candidate->get_latitude(),
                                                  candidate->get_longitude());
      bool is_dup = false;
      for (int j = i - 1; j >= 0; --j) {
        const tCity* better_city = cities_in_same_country[j];
        // We cannot evaluate without lat long.
        if (!better_city->has_lat_long()) continue;

        LatLong better_latlong = LatLong::Create(better_city->get_latitude(),
                                                 better_city->get_longitude());

        if (LatLong::SurfaceDistance(better_latlong, candidate_latlong) >
            gFlag_region_cities_dedup_min_distance) continue;

        // Check name.
        if (abs(static_cast<int>(candidate->name.size() -
                                 better_city->name.size())) >
            gFlag_region_cities_dedup_min_name_edit_distance) continue;

        if (strutil::EditDistance(candidate->name, better_city->name) >
            gFlag_region_cities_dedup_min_name_edit_distance) continue;

        // The two cities appear to be the same. Lets remove the candidate.
        is_dup = true;
        VLOG(3) << "City: " << candidate->name << " is a duplicate of "
               <<  better_city->name;
        break;
      }
      if (is_dup)
        duplicates.insert(candidate);
    }
  }

  auto end = remove_if(cities->begin(), cities->end(),
                       [&duplicates](const tCity* city) -> bool {
                           return duplicates.find(city) != duplicates.end();
                       });
  if (end != cities->end())  cities->resize(end - cities->begin());
}

// convenience wrapper for NormalizeAndLookup()
const tCity *FindCity(const string& city_name,
                      const string& state_code,
                      const string& country_code,
                      double lat, double lon,
                      double max_distance_error) {

  const auto candidates = ::region_data::Cities::Instance().NormalizeAndLookup(
      city_name, state_code, country_code);

  const auto search_latlong = LatLong::Create(lat, lon);

  const auto city_dist = region_data::Cities::Instance().GetNearestTo(
        search_latlong, candidates);

  if (!city_dist.first || city_dist.second > max_distance_error) {
    return nullptr;
  }

  return city_dist.first;
}

// Register the cities with the enity manager.
auto reg_em_cities = ::entity::EntityManager::bind("em_cities", "",
    InitializeConfigureConstructor<Cities, string>());

}  // namespace region_data

// Init after scored_cities, before code translator.
INIT_ADD("cities", 0, []{ region_data::Cities::Instance(); });
