// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_ENTITY_UTILS_ENTITY_UTILS_H_
#define _UTIL_ENTITY_UTILS_ENTITY_UTILS_H_

#include "base/defs.h"

#include "util/entity/entity_type.h"
#include "util/templates/container_util.h"

namespace entity {

// Returns the Geo precision for the a given entity.
// The lower the number, the lower the precision.
inline int GetEntityRegionPrecision(const EntityType& type) {
  static const unordered_map<EntityType, int> entity_type_precision_map = {
        // Geo
        {kEntityTypeCity, 3},
        {kEntityTypeCountry, 1},
        {kEntityTypeNeighborhood, 4},
        {kEntityTypeState, 2},

        // POIs.
        {kEntityTypeAirport, 10},  // Represents precise location but not a pinpoint region.
        {kEntityTypeAttraction, 12},  // Represents precise location but not a pinpoint region.
        {kEntityTypeCityCenter, 5},

        // Bookable
        {kEntityTypeHotel, 20},  // Represents exact value.
        {kEntityTypeVacationRental, 15},  // Represents exact-ish value.

        // Unknown
        {kEntityTypeGeoCode, 10},
      };

  return ::util::tl::FindWithDefault(entity_type_precision_map, type, 0);
}

}  // namespace entity

#endif  // _UTIL_ENTITY_UTILS_ENTITY_UTILS_H_
