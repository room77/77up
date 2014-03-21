// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/region_data/mock/cities_mock.h"

namespace {

const char kEmMockCitiesId[] = "em_mock_cities";

}  // namespace

namespace region_data {
namespace test {

MockCities& MockCitiesInstance() {
  // Set the cities_id flag to the make it instantiate the mock instance.
  gFlag_em_cities_id = kEmMockCitiesId;
  return dynamic_cast<MockCities&>(Cities::Instance());
}

region_data::tCity GetMockCity(const string& name, double lat, double lon) {
  region_data::tCity city;
  city.name = name;
  city.eid = "c/" + name;
  city.lat = lat;
  city.lon = lon;
  city.country_iso_code_2char = "US";
  city.state_iso_code_short = "CA";
  return city;
}

// Register the cities with the enity manager.
auto reg_em_mock_cities = ::entity::EntityManager::bind(kEmMockCitiesId, "",
    InitializeConfigureConstructor<MockCities, string>());

}  // namespace test
}  // namespace region_data
