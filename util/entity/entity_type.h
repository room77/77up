// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines all the types known to us.
// Every entity type must have two unique properties.
// 1. A unique enum type in EntityType.
// 2. A unique string that can be used as a prefix to create an entity id.
// e.g. for cities it is kEntityTypeCity and "c".

// NOTE: While adding a new entity make sure you add both mappings, i.e.
// 1. EntityType -> prefix  (GetEntityPrefixForType)
// 1. prefix -> EntityType  (GetEntityTypeFromPrefix)

#ifndef _UTIL_ENTITY_ENTITY_TYPE_H_
#define _UTIL_ENTITY_ENTITY_TYPE_H_

#include <unordered_map>

#include "base/defs.h"
#include "util/string/strutil.h"
#include "util/templates/container_util.h"

namespace entity {

// The different source types for a completion.
enum {
  kEntityTypeInvalid,

  // Regions.
  kEntityTypeRegionBegin = 1,

  // Geo Types.
  kEntityTypeGeoBegin = 1,
  kEntityTypeCity,
  kEntityTypeCountry,
  kEntityTypeNeighborhood,
  kEntityTypeState,
  kEntityTypeGeoEnd,

  // POIs.
  kEntityTypePOIBegin = 101,
  kEntityTypeAirport,
  kEntityTypeAttraction,
  kEntityTypeCityCenter,
  // This is for all the items that are geocoded by the front end.
  // Keep in sync with php/config/config.inc
  kEntityTypeGeoCode,
  kEntityTypePOIEnd,

  // Bookable types.
  kEntityTypeBookableBegin = 201,
  kEntityTypeHotel,
  kEntityTypeVacationRental,
  kEntityTypeBookableEnd,
  kEntityTypeRegionEnd,

  // Different attributes.
  kEntityTypeAttributeBegin = 501,
  kEntityTypeAmenity,
  kEntityTypeCategory,
  kEntityTypeFilter,
  kEntityTypeRanker,
  kEntityTypeTrustYouCategory,
  kEntityTypeAttributeEnd,

  kEntityTypeMiscBegin = 1001,
  kEntityTypeEvent,
  kEntityTypeOperator,  // When the entity is an operator.
  kEntityTypeMisc = 1099,  // This is for misc random stuff.
  kEntityTypeMiscEnd,

  // Unknown entities. These are types that are not necessarily known to the backend.
  kEntityTypeUnknownBegin = 1100,

  kEntityTypeUnknownEnd,

  // This is the last value to specify the end of the enum range.
  kEntityTypeEnd,
};
typedef uint16_t EntityType;

// Returns the prefix for the type.
// Returns "x" if the type is unknown.
inline const string& GetEntityPrefixForType(const EntityType& type) {
  static const unordered_map<EntityType, string> entity_type_map = {
        // Geo
        {kEntityTypeCity, "c"},
        {kEntityTypeCountry, "y"},
        {kEntityTypeNeighborhood, "n"},
        {kEntityTypeState, "s"},

        // POIs.
        {kEntityTypeAirport, "ap"},
        {kEntityTypeAttraction, "a"},
        {kEntityTypeCityCenter, "cc"},
        {kEntityTypeGeoCode, "g"},  // Keep in sync with php/config/config.inc

        // Bookable
        {kEntityTypeHotel, "h"},
        {kEntityTypeVacationRental, "v"},

        // Attributes.
        {kEntityTypeAmenity, "am"},
        {kEntityTypeCategory, "cat"},
        {kEntityTypeFilter, "f"},
        {kEntityTypeRanker, "r"},
        {kEntityTypeTrustYouCategory, "tyc"},

        // Misc.
        {kEntityTypeEvent, "e"},
        {kEntityTypeOperator, "o"},
        {kEntityTypeMisc, "m"},

        // Unknown
      };

  static const string kInvalid = "x";
  return ::util::tl::FindWithDefault(entity_type_map, type, kInvalid);
}

// Returns the type for the prefix.
// Returns kEntityTypeInvalid if the type is unknown.
inline EntityType GetEntityTypeFromPrefix(const string& prefix) {
  static const unordered_map<string, EntityType> entity_type_map = {
        // Geo
        {"c", kEntityTypeCity},
        {"y", kEntityTypeCountry},
        {"n", kEntityTypeNeighborhood},
        {"s", kEntityTypeState},

        // POIs.
        {"ap", kEntityTypeAirport},
        {"a", kEntityTypeAttraction},
        {"cc", kEntityTypeCityCenter},
        {"g", kEntityTypeGeoCode},  // Keep in sync with php/config/config.inc

        // Bookable
        {"h", kEntityTypeHotel},
        {"v", kEntityTypeVacationRental},

        // Attributes.
        {"am", kEntityTypeAmenity},
        {"cat", kEntityTypeCategory},
        {"f", kEntityTypeFilter},
        {"r", kEntityTypeRanker},

        // Misc.
        {"e", kEntityTypeEvent},
        {"o", kEntityTypeOperator},
        {"m", kEntityTypeMisc},

        // Unknown
      };

  return ::util::tl::FindWithDefault(entity_type_map, prefix, kEntityTypeInvalid);
}

// Different utility functions to identify a type.
inline bool IsTypeRegion(EntityType entity_type) {
  return kEntityTypeRegionBegin < entity_type && entity_type < kEntityTypeRegionEnd;
}

inline bool IsTypeGeo(EntityType entity_type) {
  return kEntityTypeGeoBegin < entity_type && entity_type < kEntityTypeGeoEnd;
}

inline bool IsTypePOI(EntityType entity_type) {
  return kEntityTypePOIBegin < entity_type && entity_type < kEntityTypePOIEnd;
}

inline bool IsTypeBookable(EntityType entity_type) {
  return kEntityTypeBookableBegin < entity_type && entity_type < kEntityTypeBookableEnd;
}

inline bool IsTypeAttribute(EntityType entity_type) {
  return kEntityTypeAttributeBegin < entity_type && entity_type < kEntityTypeAttributeEnd;
}

inline bool IsTypeMisc(EntityType entity_type) {
  return kEntityTypeMiscBegin < entity_type && entity_type < kEntityTypeMiscEnd;
}

inline bool IsTypeUnknown(EntityType entity_type) {
  return kEntityTypeUnknownBegin < entity_type && entity_type < kEntityTypeUnknownEnd;
}

// TODO are these necessary?
inline bool IsTypeNeighborhood(EntityType entity_type) {
  return kEntityTypeNeighborhood == entity_type;
}

inline bool IsTypeHotel(EntityType entity_type) {
  return kEntityTypeHotel == entity_type;
}

}  // namespace entity


#endif  // _UTIL_ENTITY_ENTITY_TYPE_H_
