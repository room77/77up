// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/region_data/attractions.h"

#include "util/init/init.h"
#include "util/string/strutil.h"
#include "util/region_data/cities.h"
#include "util/region_data/countries.h"
#include "util/region_data/region_utils.h"
#include "util/region_data/states.h"

FLAG_string(em_attractions_id, "em_attractions",
            "The default entity manager id for attractions.");

FLAG_string(region_attractions_file,
            "static_data/push/auto/region_data/attractions/attractions.csv",
            "List of attractions.");

namespace region_data {

tAttractionReply attraction_storage_to_reply::operator() (
    const tAttraction& attr) const {
  tAttractionReply reply = super::operator()(attr);

  // City Info.
  reply.city = attr.city;

  // State Info.
  if (!attr.state_iso_code_short.empty()) {
    reply.state_code = attr.state_iso_code_short;
    const tState* s = States::Instance().LookupByCode(
        attr.state_iso_code_short, attr.country_iso_code_2char);
    if (s != nullptr) reply.state_name = s->name;
  }

  // Country info.
  if (!attr.country_iso_code_2char.empty()) {
    reply.country_code = attr.country_iso_code_2char;
    const tCountry* c = Countries::Instance().LookupByCode(
        attr.country_iso_code_2char);
    if (c != nullptr) reply.country_name = c->name;
  }

  reply.classification = attr.classification;

  return reply;
}

bool order_attractions_by_name::operator() (const tAttraction& a1,
                                            const tAttraction& a2) const {
  int p1 = I18N::ListPriority(a1.country_iso_code_2char);
  int p2 = I18N::ListPriority(a2.country_iso_code_2char);

  // Order by priority.
  if (p1 != p2) return p1 < p2;

  // Order by attraction name.
  if (a1.name != a2.name) return a1.name < a2.name;

  // Order attractions with same name by state.
  if (a1.state_iso_code_short != a2.state_iso_code_short)
    return a1.state_iso_code_short < a2.state_iso_code_short;

  // Order attractions with same name, state by country.
  if (a1.country_iso_code_2char != a2.country_iso_code_2char)
    return a1.country_iso_code_2char < a2.country_iso_code_2char;

  // Break ties arbitrarily.
  return true;
}

bool Attractions::Initialize() {
  // Index by name, prefix and latlong.
  super::ConfigParams params;
  params.file = gFlag_region_attractions_file;
  params.build_name_index = true;
  params.name_index_is_unique = false;
  params.build_latlong_index = true;
  params.build_entity_id_index = true;

  ASSERT(super::Initialize(params));
  return true;
}

int Attractions::LookupByNameEx(const string& name, const string& city,
    const string& state_code, const string& country_code,
    vector<const tAttraction*>* result, int max_results) const {

  VLOG(3) << "Req: " << name << ", " << city << ", " << state_code << ", " << country_code;

  // Get all candidates by name.
  if (!LookupByName(name, result, max_results)) return 0;

  VLOG(3) << "Found " << result->size() << " candidates. "
      << serial::Serializer::ToJSON(*result, {1, 1});

  // Get the country code.
  string parsed_country_code;
  string norm_country_code;
  if (country_code.size()) {
    const tCountry* c = Countries::Instance().LookupByCodeMatchAny(country_code);

    if (c != nullptr) parsed_country_code = c->iso_code_2char;
    else norm_country_code = utils::NormalizeString(country_code);
  }

  string parsed_state_code;
  string norm_state_code;
  if (state_code.size()) {
    if (parsed_country_code.size()) {
      const tState* s = States::Instance().LookupByCode(state_code, parsed_country_code);
      if (s != nullptr) parsed_state_code = s->iso_code_short;
    }
    if (parsed_state_code.empty()) norm_state_code = utils::NormalizeString(state_code);
  }

  string parsed_city_eid;
  string norm_city_name;
  if (city.size()) {
    if (parsed_country_code.size()) {
      const tCity* c = Cities::Instance().LookupByNameAndCode(city, parsed_state_code,
                                                              parsed_country_code);
      if (c != nullptr) parsed_city_eid = c->eid;
    }
    if (parsed_city_eid.empty()) norm_city_name = utils::NormalizeString(city);
  }

  VLOG(3) << "Parsed: " << parsed_city_eid << ", " << parsed_state_code << ", "
          << parsed_country_code;

  VLOG(3) << "Norm: " << norm_city_name << ", " << norm_state_code << ", " << norm_country_code;

  auto iter = remove_if(result->begin(), result->end(), [&](const tAttraction* attr) -> bool {
    country_code_matcher cc_matcher;
    // Remove if the country name is not the same.
    if (attr->country_iso_code_2char.size()) {
      if (parsed_country_code.size()) {
        if (!cc_matcher(attr->country_iso_code_2char, parsed_country_code)) return true;
      } else if (norm_country_code.size()) {
        if (!cc_matcher(utils::NormalizeString(attr->country_iso_code_2char), norm_country_code))
          return true;
      }
    }

    if (attr->state_iso_code_short.size()) {
      if (parsed_state_code.size()) {
        if (attr->state_iso_code_short != parsed_state_code) return true;
      } else if (norm_state_code.size()) {
        if (utils::NormalizeString(attr->state_iso_code_short) != norm_state_code) return true;
      }
    }

    bool checked_city = false;
    if (attr->city_eid.size()) {
      if (parsed_city_eid.size()) {
        checked_city = true;
        if (attr->city_eid != parsed_city_eid) return true;
      }
    }

    if (!checked_city && attr->city.size()) {
      if (norm_city_name.size()) {
        if (utils::NormalizeString(attr->city) != norm_city_name) return true;
      }
    }

    return false;
  });

  // Resize if necessary
  if (iter != result->end()) result->resize(iter - result->begin());

  VLOG(3) << "Trimmed to " << result->size() << " candidates. "
      << serial::Serializer::ToJSON(*result, {1, 1});

  return result->size();
}

// Register the attractions with the enity manager.
auto reg_em_attractions = ::entity::EntityManager::bind("em_attractions", "",
    InitializeConfigureConstructor<Attractions, string>());

}  // namespace region_data

// Init before code translator.
INIT_ADD("attractions", 0, []{ region_data::Attractions::Instance(); });
