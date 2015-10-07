// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_REGION_DATA_MOCK_REGION_MOCK_H_
#define _UTIL_REGION_DATA_MOCK_REGION_MOCK_H_

#include "util/region_data/region.h"

#include "test/cc/test_main.h"

namespace region_data {
namespace test {

template<typename Data>
class MockRegion : public Region<Data> {
  typedef Region<Data> super;
  using Comparator = typename super::Comparator;
  using ConfigParams = typename super::ConfigParams;

 public:
  // By default return true.
  bool Configure(const string& opts) { return true; }
  bool Initialize() { return true; }

  MOCK_CONST_METHOD1_T(LookupUniqueByEntityId,
                       const Data*(const string& entity_id));

  MOCK_CONST_METHOD4_T(LookupByName,
      int(const string& name, vector<const Data*>* result, int max_results,
          const Comparator& comp));

  MOCK_CONST_METHOD1_T(LookupUniqueByName,
                       const Data*(const string& name));

  MOCK_CONST_METHOD4_T(LookupByLatLong,
      int(const LatLong& reference_point,
          vector<pair<const Data*, double> >* result,
          int max_results, double max_radius_km));

  MOCK_CONST_METHOD1_T(FindByLatLong,
                       const Data*(const LatLong& reference_point));

  MOCK_CONST_METHOD4_T(Parse,
      int(const string& input, vector<const Data*>* result, int max_results,
          const Comparator& comp));

  MOCK_CONST_METHOD1_T(Dedup,
                     void(vector<const Data*>* result));

  MOCK_CONST_METHOD3_T(SortResults,
      void(vector<const Data*>* result, int max_results,
          const Comparator& comp));

 protected:
  MOCK_METHOD1_T(Initialize,
                 bool(const ConfigParams& params));
};

struct tMockRegion : public tRegion {
  explicit tMockRegion(entity::EntityType type = entity::kEntityTypeCity) : type(type) {}

  virtual entity::EntityType GetEntityType() const { return type; }

  entity::EntityType type;
};

tMockRegion GetMockRegion(const string& name = "test", double lat = 21, double lon = 23);

}  // namespace test
}  // namespace region_data


#endif  // _UTIL_REGION_DATA_MOCK_REGION_MOCK_H_
