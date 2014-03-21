// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/region_data/region_utils.h"

#include <limits>
#include <utility>

#include "util/geo/latlong.h"
#include "util/region_data/attractions.h"
#include "util/region_data/cities.h"
#include "util/region_data/neighborhood/neighborhoods.h"

FLAG_string(region_current_location_eid, "g/cur_loc",
             "The entity ID used for the user's current location");

namespace region_data {
namespace utils {
namespace {

// Returns the city id for the name.
// If the name is ambiguous, the one closest to the provided lat/lon is returned.
template<typename RD, typename Data = typename RD::DataType>
string GetIdFromName(const RD& rd, const string& name, double lat = 0, double lon = 0,
                     double* distance = nullptr) {
  if (name.empty()) return "";

  // Lookpup the region by name
  vector<const Data*> items;
  rd.LookupByName(name, &items);

  if (items.empty()) return "";

  // We have no way to disambiguate, return the one that is at the top.
  if (lat == 0 && lon == 0) return items[0]->eid;

  pair<const Data*, double> nearest = rd.GetNearestTo(LatLong::Create(lat, lon), items);
  if (nearest.first != nullptr) {
    if (distance != nullptr) *distance = nearest.second;
    return nearest.first->eid;
  }
  return "";
}

}  // namespace

string GetCityIdFromName(const string& name, double lat, double lon, double* distance) {
  return GetIdFromName(Cities::Instance(), name, lat, lon, distance);
}

string GetNeighborhoodIdFromName(const string& name, double lat, double lon, double* distance) {
  return GetIdFromName(Neighborhoods::Instance(), name, lat, lon, distance);
}

string GetAttractionIdFromName(const string& name, double lat, double lon, double* distance) {
  return GetIdFromName(Attractions::Instance(), name, lat, lon, distance);
}

// Returns the lat long for the name.
// If the name is ambiguous, the one closest to the provided lat/lon is returned.
string GetLocationIdFromName(const string& name, double lat, double lon, double* distance) {
  if (name.empty()) return "";
  // Special check for current location.
  if (name == "Current Location" || name == "Near Me") {
    return gFlag_region_current_location_eid;
  }

  // First try cities.
  string location_id = GetCityIdFromName(name, lat, lon, distance);
  if (location_id.size()) return location_id;

  // Second try neigborhoods.
  location_id = GetNeighborhoodIdFromName(name, lat, lon, distance);
  if (location_id.size()) return location_id;

  location_id = GetAttractionIdFromName(name, lat, lon, distance);
  if (location_id.size()) return location_id;

  return "";
}

}  // namespace utils
}  // namespace region_data
