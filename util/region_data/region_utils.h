// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_REGION_DATA_REGION_UTILS_H_
#define _UTIL_REGION_DATA_REGION_UTILS_H_

#include "base/defs.h"

namespace region_data {
namespace utils {

// Returns the attraction id for the name. If a lat long is provided and there are multiple
// attractions available with a given name, the one closest to the lat long is returned.
string GetAttractionIdFromName(const string& name, double lat = 0, double lon = 0,
                               double* distance = nullptr);

// Returns the city id for the name. If a lat long is provided and there are multiple
// cities available with a given name, the one closest to the lat long is returned.
string GetCityIdFromName(const string& name, double lat = 0, double lon = 0,
                         double* distance = nullptr);

// Returns the neigborhood id for the name. If a lat long is provided and there are multiple
// neigborhoods available with a given name, the one closest to the lat long is returned.
string GetNeighborhoodIdFromName(const string& name, double lat = 0, double lon = 0,
                                 double* distance = nullptr);

// Returns the location id for the name. If a lat long is provided and there are multiple
// location available with a given name, the one closest to the lat long is returned.
// The name is compared to locations in the following order:
// 1. Cities.
// 2. Neighborhoods.
// 3. Attractions.
string GetLocationIdFromName(const string& name, double lat = 0, double lon = 0,
                             double* distance = nullptr);

}  // namespace utils
}  // namespace region_data


#endif  // _UTIL_REGION_DATA_REGION_UTILS_H_
