#include "util/region_data/neighborhoods.h"

#include "util/init/init.h"
#include "util/file/field_reader.h"
#include "util/region_data/neighborhood/neighborhoods.h"

FLAG_int(region_data_neighborhood_max_city_error,
         50,
         "Maximum lat/lon error allowed for matching neighborhoods to cities.");

FLAG_int(region_data_neighborhood_min_num_hotels,
         1,
         "Minimum number of hotels for a neighborhood to be in the"
             " Neighborhoods collection");

FLAG_string(region_data_neighborhood_blacklist,
            "static_data/dev_only/region_data/neighborhoods"
                "/neighborhood_blacklist.csv",
            "The neighborhoods that are duplicates of attractions");

FLAG_bool(region_data_use_neighborhood_blacklist,
          true,
          "If set to true, do not exclude neighborhoods in the blacklist.");

//FLAG_string(neighborhood_city_override_file,
//            "static_data/dev_only/region_data/neighborhoods/"
//                "neighborhood_city_override.csv",
//            "CSV file for neighborhood city overriding");

FLAG_string(utils_neighborhoods_id, "utils_neighborhoods",
            "The default id for region_data::utils::Neighborhoods");

namespace region_data { namespace utils {

namespace {

#if 0
unordered_map<string, string> LoadNeighborhoodCityOverrideTable() {
  unordered_map<string, string> ret;

  EscapedDelimitedFieldReader r(',');
  ifstream file(gFlag_neighborhood_city_override_file);
  ASSERT(file.good()) << "cannot open file: "
                      << gFlag_neighborhood_city_override_file;
  r.ParseTags(file);
  while (r.ParseLine(file)) {
    string neighborhood_eid;
    string city_eid;
    r.Get("eid", &neighborhood_eid);
    r.Get("city_eid", &city_eid);
    ASSERT(!neighborhood_eid.empty());
    ASSERT(!city_eid.empty());

    const auto insert_pair = ret.emplace(neighborhood_eid, city_eid);
    ASSERT(insert_pair.second);
  }
  return ret;
}

#endif

unordered_set<string> LoadNeighborhoodBlacklist() {
  unordered_set<string> ret;

  EscapedDelimitedFieldReader r(',');
  ifstream file(gFlag_region_data_neighborhood_blacklist);
  ASSERT(file.good()) << "cannot open "
                      << gFlag_region_data_neighborhood_blacklist;
  r.ParseTags(file);
  while (r.ParseLine(file)) {
    string neighborhood_eid;
    r.Get("eid", &neighborhood_eid);
    ASSERT(!neighborhood_eid.empty());

    const auto insert_pair = ret.insert(neighborhood_eid);
    ASSERT(insert_pair.second);
  }
  LOG(INFO) << "read " << ret.size() << " blacklisted eid's.";
  return ret;
}

}  // namespace unnamed

const Neighborhoods& Neighborhoods::Instance() {
  struct Creator {
    Neighborhoods *operator()() {
      mutable_shared_proxy _ = make_shared(gFlag_utils_neighborhoods_id);
      ASSERT_NOTNULL(_);
      pin(_);
      return dynamic_cast<Neighborhoods *>(_.get());
    }
  };
  static const Neighborhoods& _ = *Creator()();
  return _;
}

bool Neighborhoods::Initialize() {
  struct MakeFullCityName {
    string operator()(const ::region_data::tNeighborhood& n) {
      return strutil::Join(
          {n.city, n.state_iso_code_short, n.country_iso_code_2char}, ", ");
    }
  };

//  const auto neighborhood_city_override_map =
//      LoadNeighborhoodCityOverrideTable();

  const auto neighborhood_blacklist = LoadNeighborhoodBlacklist();
  set<string> unmatched_cities;

  for (const auto& sp_neighborhood_info :
      ::region_data::Neighborhoods::Instance().storage()) {

#if 0
    const region_data::tCity *p_city = nullptr;
    const auto override_city_eid = ::util::tl::FindWithDefault(
        neighborhood_city_override_map, sp_neighborhood_info->eid, "");
    if (!override_city_eid.empty()) {
      p_city = Cities::Instance().LookupUniqueByEntityId(override_city_eid);
      LOG(INFO) << "override the city: " << sp_neighborhood_info->ToJSON({1, 1})
                << "\t" << p_city->ToJSON({1, 1});
      ASSERT_NOTNULL(p_city)
          << "unknown city_eid: " << override_city_eid;
    } else {
      p_city = FindCity(*sp_neighborhood_info);
    }
#endif

    // exclude neighborhoods that have too few hotels
    const auto& hotel_count = sp_neighborhood_info->loc.hotel_count;
    if (hotel_count < gFlag_region_data_neighborhood_min_num_hotels) continue;

    // exclude neighborhoods that have the same name as the city name
    if (NormalizeString(sp_neighborhood_info->loc.name)
        == NormalizeString(sp_neighborhood_info->loc.mcd)) {
      LOG(INFO) << "drop: " << sp_neighborhood_info->ToJSON(-1);
      continue;
    }

    // exclude neighborhoods that are in the blacklist
    if (gFlag_region_data_use_neighborhood_blacklist
        && ::util::tl::Contains(neighborhood_blacklist,
                                sp_neighborhood_info->eid)) {
      continue;
    }

    const auto *p_city = FindCity(*sp_neighborhood_info);
    if (p_city) {
      storage_.emplace_back(
          ::std::make_shared<tNeighborhood>(*sp_neighborhood_info));
      storage_.back()->city_eid = p_city->eid;
      ASSERT_GE(sp_neighborhood_info->loc.zoom_level, 4);
    } else if (!sp_neighborhood_info->loc.mcd.empty()) {
      unmatched_cities.emplace(
          MakeFullCityName()(tNeighborhood(*sp_neighborhood_info)));
    }
  }

#if 0
  for (const auto& sp_neighborhood : storage_) {
    const auto insert_pair =
        index_by_eid_.emplace(sp_neighborhood->eid, sp_neighborhood.get());
    ASSERT(insert_pair.second);
  }
#endif

  LOG(INFO) << unmatched_cities.size() << " unmatched cities: "
            << serial::Serializer::ToJSON(unmatched_cities, {1, 1});

  BuildEntityIdIndex();

  return true;
}

void NeighborhoodExample() {
  int count = 0;
  for (const auto& sp_neighborhood : Neighborhoods::Instance().storage()) {
    tNeighborhood neighborhood(*sp_neighborhood);
    LOG(INFO) << neighborhood.ToJSON(-1);
    if (count++ >= 10) break;
  }
}

bool order_neighborhoods_by_name::operator()(const tNeighborhood& n1,
                                             const tNeighborhood& n2) const {
  int p1 = I18N::ListPriority(n1.country_iso_code_2char);
  int p2 = I18N::ListPriority(n2.country_iso_code_2char);

  // Order by priority.
  if (p1 != p2) return p1 < p2;

  // Order by attraction name.
  if (n1.name != n2.name) return n1.name < n2.name;

  // Order attractions with same name by state.
  if (n1.state_iso_code_short != n2.state_iso_code_short)
    return n1.state_iso_code_short < n2.state_iso_code_short;

  // Order attractions with same name, state by country.
  if (n1.country_iso_code_2char != n2.country_iso_code_2char)
    return n1.country_iso_code_2char < n2.country_iso_code_2char;

  // Break ties arbitrarily.
  return true;
}

}  // namespace region_data::utils

tNeighborhood::tNeighborhood(const region_data::tNeighborhoodInfoWithBounds& n)
    : tRegion(n) {
  const auto& loc = n.loc;
  name = loc.name;
  city = loc.mcd;
  state_iso_code_short = loc.state;
  country_iso_code_2char = loc.country_code;

  zoom_level = loc.zoom_level;
}

const tCity *FindCity(const tNeighborhoodInfoWithBounds& neighborhood) {

  const auto& loc = neighborhood.loc;
  const string& city_name = loc.mcd;
  const string& state_code = loc.state;
  const string& country_code = loc.country_code;

  ASSERT_DEV_EQ(country_code, "US");

  if (state_code.empty() || country_code.empty()) return nullptr;

  const auto *p_city = ::region_data::FindCity(
      city_name,
      state_code,
      country_code,
      neighborhood.lat,
      neighborhood.lon,
      gFlag_region_data_neighborhood_max_city_error);

  if (!p_city) return nullptr;

  if (state_code != p_city->state_iso_code_short
      || country_code != p_city->country_iso_code_2char) {

    LOG(WARNING)
        << "match between (" << city_name << ", " << state_code << ", "
        << country_code << ") and " << p_city->ToJSON(-1);
    return nullptr;
  }

  return p_city;
}

}  // namespace region_data

auto reg_region_data_utils_neighborhoods =
//    ::entity::EntityManager::bind(
    ::region_data::utils::Neighborhoods::bind(
        gFlag_utils_neighborhoods_id,
        "",
        InitializeConfigureConstructor<::region_data::utils::Neighborhoods,
                                       string>());

INIT_ADD("utils_neighborhoods", 0, [] {
  ::region_data::utils::Neighborhoods::Instance();
});
