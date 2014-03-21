// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "util/entity/entity_type.h"

#include "test/cc/test_main.h"

namespace entity {
namespace test {

TEST(EntityIdTest, GetEntityPrefixForType) {
  // Geo
  EXPECT_EQ("c", GetEntityPrefixForType(kEntityTypeCity));
  EXPECT_EQ("y", GetEntityPrefixForType(kEntityTypeCountry));
  EXPECT_EQ("n", GetEntityPrefixForType(kEntityTypeNeighborhood));
  EXPECT_EQ("s", GetEntityPrefixForType(kEntityTypeState));

  // POIs.
  EXPECT_EQ("ap", GetEntityPrefixForType(kEntityTypeAirport));
  EXPECT_EQ("a", GetEntityPrefixForType(kEntityTypeAttraction));
  EXPECT_EQ("cc", GetEntityPrefixForType(kEntityTypeCityCenter));
  EXPECT_EQ("g", GetEntityPrefixForType(kEntityTypeGeoCode));

  // Bookable
  EXPECT_EQ("h", GetEntityPrefixForType(kEntityTypeHotel));
  EXPECT_EQ("v", GetEntityPrefixForType(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_EQ("am", GetEntityPrefixForType(kEntityTypeAmenity));
  EXPECT_EQ("cat", GetEntityPrefixForType(kEntityTypeCategory));
  EXPECT_EQ("f", GetEntityPrefixForType(kEntityTypeFilter));
  EXPECT_EQ("r", GetEntityPrefixForType(kEntityTypeRanker));

  // Misc.
  EXPECT_EQ("e", GetEntityPrefixForType(kEntityTypeEvent));
  EXPECT_EQ("o", GetEntityPrefixForType(kEntityTypeOperator));
  EXPECT_EQ("m", GetEntityPrefixForType(kEntityTypeMisc));

  // Unknown.
}

TEST(EntityIdTest, GetEntityTypeFromPrefix) {
  // Geo
  EXPECT_EQ(kEntityTypeCity, GetEntityTypeFromPrefix("c"));
  EXPECT_EQ(kEntityTypeCountry, GetEntityTypeFromPrefix("y"));
  EXPECT_EQ(kEntityTypeNeighborhood, GetEntityTypeFromPrefix("n"));
  EXPECT_EQ(kEntityTypeState, GetEntityTypeFromPrefix("s"));

  // POIs.
  EXPECT_EQ(kEntityTypeAirport, GetEntityTypeFromPrefix("ap"));
  EXPECT_EQ(kEntityTypeAttraction, GetEntityTypeFromPrefix("a"));
  EXPECT_EQ(kEntityTypeCityCenter, GetEntityTypeFromPrefix("cc"));
  EXPECT_EQ(kEntityTypeGeoCode, GetEntityTypeFromPrefix("g"));

  // Bookable
  EXPECT_EQ(kEntityTypeHotel, GetEntityTypeFromPrefix("h"));
  EXPECT_EQ(kEntityTypeVacationRental, GetEntityTypeFromPrefix("v"));

  // Attributes.
  EXPECT_EQ(kEntityTypeAmenity, GetEntityTypeFromPrefix("am"));
  EXPECT_EQ(kEntityTypeCategory, GetEntityTypeFromPrefix("cat"));
  EXPECT_EQ(kEntityTypeFilter, GetEntityTypeFromPrefix("f"));
  EXPECT_EQ(kEntityTypeRanker, GetEntityTypeFromPrefix("r"));

  // Misc.
  EXPECT_EQ(kEntityTypeEvent, GetEntityTypeFromPrefix("e"));
  EXPECT_EQ(kEntityTypeOperator, GetEntityTypeFromPrefix("o"));
  EXPECT_EQ(kEntityTypeMisc, GetEntityTypeFromPrefix("m"));

  // Unknown.
}

TEST(EntityIdTest, IsTypeRegion) {
  // Geo
  EXPECT_TRUE(IsTypeRegion(kEntityTypeCity));
  EXPECT_TRUE(IsTypeRegion(kEntityTypeCountry));
  EXPECT_TRUE(IsTypeRegion(kEntityTypeNeighborhood));
  EXPECT_TRUE(IsTypeRegion(kEntityTypeState));

  // POIs.
  EXPECT_TRUE(IsTypeRegion(kEntityTypeAirport));
  EXPECT_TRUE(IsTypeRegion(kEntityTypeAttraction));
  EXPECT_TRUE(IsTypeRegion(kEntityTypeCityCenter));
  EXPECT_TRUE(IsTypeRegion(kEntityTypeGeoCode));

  // Bookable
  EXPECT_TRUE(IsTypeRegion(kEntityTypeHotel));
  EXPECT_TRUE(IsTypeRegion(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_FALSE(IsTypeRegion(kEntityTypeAmenity));
  EXPECT_FALSE(IsTypeRegion(kEntityTypeCategory));
  EXPECT_FALSE(IsTypeRegion(kEntityTypeFilter));
  EXPECT_FALSE(IsTypeRegion(kEntityTypeRanker));

  // Misc.
  EXPECT_FALSE(IsTypeRegion(kEntityTypeEvent));
  EXPECT_FALSE(IsTypeRegion(kEntityTypeOperator));
  EXPECT_FALSE(IsTypeRegion(kEntityTypeMisc));

  // Unknown.
}

TEST(EntityIdTest, IsTypeGeo) {
  // Geo
  EXPECT_TRUE(IsTypeGeo(kEntityTypeCity));
  EXPECT_TRUE(IsTypeGeo(kEntityTypeCountry));
  EXPECT_TRUE(IsTypeGeo(kEntityTypeNeighborhood));
  EXPECT_TRUE(IsTypeGeo(kEntityTypeState));

  // POIs.
  EXPECT_FALSE(IsTypeGeo(kEntityTypeAirport));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeAttraction));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeCityCenter));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeGeoCode));

  // Bookable
  EXPECT_FALSE(IsTypeGeo(kEntityTypeHotel));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_FALSE(IsTypeGeo(kEntityTypeAmenity));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeCategory));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeFilter));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeRanker));

  // Misc.
  EXPECT_FALSE(IsTypeGeo(kEntityTypeEvent));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeOperator));
  EXPECT_FALSE(IsTypeGeo(kEntityTypeMisc));

  // Unknown.
}

TEST(EntityIdTest, IsTypePOI) {
  // Geo
  EXPECT_FALSE(IsTypePOI(kEntityTypeCity));
  EXPECT_FALSE(IsTypePOI(kEntityTypeCountry));
  EXPECT_FALSE(IsTypePOI(kEntityTypeNeighborhood));
  EXPECT_FALSE(IsTypePOI(kEntityTypeState));

  // POIs.
  EXPECT_TRUE(IsTypePOI(kEntityTypeAirport));
  EXPECT_TRUE(IsTypePOI(kEntityTypeAttraction));
  EXPECT_TRUE(IsTypePOI(kEntityTypeCityCenter));
  EXPECT_TRUE(IsTypePOI(kEntityTypeGeoCode));

  // Bookable
  EXPECT_FALSE(IsTypePOI(kEntityTypeHotel));
  EXPECT_FALSE(IsTypePOI(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_FALSE(IsTypePOI(kEntityTypeAmenity));
  EXPECT_FALSE(IsTypePOI(kEntityTypeCategory));
  EXPECT_FALSE(IsTypePOI(kEntityTypeFilter));
  EXPECT_FALSE(IsTypePOI(kEntityTypeRanker));

  // Misc.
  EXPECT_FALSE(IsTypePOI(kEntityTypeEvent));
  EXPECT_FALSE(IsTypePOI(kEntityTypeOperator));
  EXPECT_FALSE(IsTypePOI(kEntityTypeMisc));

  // Unknown.
}

TEST(EntityIdTest, IsTypeBookable) {
  // Geo
  EXPECT_FALSE(IsTypeBookable(kEntityTypeCity));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeCountry));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeNeighborhood));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeState));

  // POIs.
  EXPECT_FALSE(IsTypeBookable(kEntityTypeAirport));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeAttraction));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeCityCenter));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeGeoCode));

  // Bookable
  EXPECT_TRUE(IsTypeBookable(kEntityTypeHotel));
  EXPECT_TRUE(IsTypeBookable(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_FALSE(IsTypeBookable(kEntityTypeAmenity));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeCategory));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeFilter));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeRanker));

  // Misc.
  EXPECT_FALSE(IsTypeBookable(kEntityTypeEvent));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeOperator));
  EXPECT_FALSE(IsTypeBookable(kEntityTypeMisc));

  // Unknown.
}

TEST(EntityIdTest, IsTypeAttribute) {
  // Geo
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeCity));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeCountry));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeNeighborhood));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeState));

  // POIs.
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeAirport));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeAttraction));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeCityCenter));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeGeoCode));

  // Bookable
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeHotel));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_TRUE(IsTypeAttribute(kEntityTypeAmenity));
  EXPECT_TRUE(IsTypeAttribute(kEntityTypeCategory));
  EXPECT_TRUE(IsTypeAttribute(kEntityTypeFilter));
  EXPECT_TRUE(IsTypeAttribute(kEntityTypeRanker));

  // Misc.
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeEvent));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeOperator));
  EXPECT_FALSE(IsTypeAttribute(kEntityTypeMisc));

  // Unknown.
}

TEST(EntityIdTest, IsTypeMisc) {
  // Geo
  EXPECT_FALSE(IsTypeMisc(kEntityTypeCity));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeCountry));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeNeighborhood));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeState));

  // POIs.
  EXPECT_FALSE(IsTypeMisc(kEntityTypeAirport));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeAttraction));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeCityCenter));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeGeoCode));

  // Bookable
  EXPECT_FALSE(IsTypeMisc(kEntityTypeHotel));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_FALSE(IsTypeMisc(kEntityTypeAmenity));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeCategory));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeFilter));
  EXPECT_FALSE(IsTypeMisc(kEntityTypeRanker));

  // Misc.
  EXPECT_TRUE(IsTypeMisc(kEntityTypeEvent));
  EXPECT_TRUE(IsTypeMisc(kEntityTypeOperator));
  EXPECT_TRUE(IsTypeMisc(kEntityTypeMisc));

  // Unknown.
}

TEST(EntityIdTest, IsTypeUnknown) {
  // Geo
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeCity));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeCountry));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeNeighborhood));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeState));

  // POIs.
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeAirport));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeAttraction));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeCityCenter));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeGeoCode));

  // Bookable
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeHotel));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeVacationRental));

  // Attributes.
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeAmenity));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeCategory));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeFilter));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeRanker));

  // Misc.
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeEvent));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeOperator));
  EXPECT_FALSE(IsTypeUnknown(kEntityTypeMisc));

  // Unknown.
}

}  // namespace test
}  // namespace entity
