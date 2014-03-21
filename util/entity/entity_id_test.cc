// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "util/entity/entity_id.h"

#include "test/cc/test_main.h"


namespace entity {
namespace test {

TEST(EntityIdTest, GetEntityIdFromBaseId) {
  EXPECT_EQ("c/ABCD", GetEntityIdFromBaseId(kEntityTypeCity, "ABCD"));
  EXPECT_EQ("y/US", GetEntityIdFromBaseId(kEntityTypeCountry, "US"));
  EXPECT_EQ("h/1234", GetEntityIdFromBaseId(kEntityTypeHotel, 1234));
}

TEST(EntityIdTest, GetBaseIdFromEntityId) {
  EXPECT_EQ("ABCD", GetBaseIdFromEntityId<string>("c/ABCD"));
  EXPECT_EQ("US", GetBaseIdFromEntityId<string>("y/US"));
  EXPECT_EQ(1234, GetBaseIdFromEntityId<int>("h/1234"));
}

TEST(EntityIdTest, GetEntityTypeFromEntityId) {
  EXPECT_EQ(kEntityTypeCity, GetEntityTypeFromEntityId("c/ABCD"));
  EXPECT_EQ(kEntityTypeCountry, GetEntityTypeFromEntityId("y/US"));
  EXPECT_EQ(kEntityTypeHotel, GetEntityTypeFromEntityId("h/1234"));
}

TEST(EntityIdTest, GetBaseIdAndTypeFromEntityId) {
  { string res;
    EXPECT_EQ(kEntityTypeCity, GetBaseIdAndTypeFromEntityId("c/ABCD", &res));
    EXPECT_EQ("ABCD", res);
  }

  { string res;
    EXPECT_EQ(kEntityTypeCountry, GetBaseIdAndTypeFromEntityId("y/US", &res));
    EXPECT_EQ("US", res);
  }

  { int res;
    EXPECT_EQ(kEntityTypeHotel, GetBaseIdAndTypeFromEntityId("h/1234", &res));
    EXPECT_EQ(1234, res);
  }
}

TEST(EntityIdTest, AppendEntityId) {
  string city_id = "c/ABCD";
  EXPECT_EQ("c/ABCD|y/US", AppendEntityId("y/US", &city_id));

  string hid = "h/1234";
  EXPECT_EQ("h/1234|c/ABCD|y/US", AppendEntityId(city_id, &hid));
}

}  // namespace test
}  // namespace entity
