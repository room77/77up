// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/region_data/mock/region_mock.h"

namespace region_data {
namespace test {

tMockRegion GetMockRegion(const string& name, double lat, double lon) {
  tMockRegion region;
  region.name = name;
  region.eid = "c/" + name;
  region.lat = lat;
  region.lon = lon;
  return region;
}

}  // namespace test
}  // namespace region_data
