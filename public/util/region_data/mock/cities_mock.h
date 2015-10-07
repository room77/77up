// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_REGION_DATA_MOCK_CITIES_MOCK_H_
#define _UTIL_REGION_DATA_MOCK_CITIES_MOCK_H_

#include "util/region_data/cities.h"

#include "test/cc/test_main.h"

namespace region_data {
namespace test {

class MockCities : public Cities {
  typedef Cities super;
  using Comparator = super::CompType;
  using ConfigParams = super::ConfigParams;

 public:
  // By default return true.
  virtual bool Configure(const string& opts) { return true; }
  virtual bool Initialize() { return true; }

  MOCK_CONST_METHOD1(LookupUniqueByEntityId,
                     const tCity*(const string& entity_id));

  MOCK_CONST_METHOD4(LookupByName,
      int(const string& name, vector<const tCity*>* result, int max_results,
          const Comparator& comp));

  MOCK_CONST_METHOD1(LookupUniqueByName,
                     const tCity*(const string& name));

  MOCK_CONST_METHOD4(LookupByLatLong,
      int(const LatLong& reference_point,
          vector<pair<const tCity*, double> >* result,
          int max_results, double max_radius_km));

  MOCK_CONST_METHOD3(LookupByNameAndCode,
                     const tCity *(const string& name, const string& state_code,
                                   const string& country_code));

  MOCK_CONST_METHOD1(FindByLatLong,
                     const tCity*(const LatLong& reference_point));

  MOCK_CONST_METHOD4(Parse,
      int(const string& input, vector<const tCity*>* result, int max_results,
          const Comparator& comp));

  MOCK_CONST_METHOD1(Dedup,
                     void(vector<const tCity*>* result));

  MOCK_CONST_METHOD3(SortResults,
      void(vector<const tCity*>* result, int max_results,
          const Comparator& comp));

 protected:
  MOCK_METHOD1(Initialize,
               bool(const ConfigParams& params));
};

// Returns the mock instance for the city.
// Note. This function should be called before anyone calls Cities::Instance() directly.
// Otherwise this will not work.
MockCities& MockCitiesInstance();

// Returns a mock city using a given city name.
region_data::tCity GetMockCity(const string& name = "test", double lat = 21, double lon = 23);

}  // namespace test
}  // namespace region_data


#endif  // _UTIL_REGION_DATA_MOCK_CITIES_MOCK_H_
