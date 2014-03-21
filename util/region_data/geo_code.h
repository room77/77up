// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Geo code that can be passed from the front end.

#ifndef _UTIL_REGION_DATA_GEO_CODE_H_
#define _UTIL_REGION_DATA_GEO_CODE_H_

#include "util/region_data/region.h"

namespace region_data {

struct tGeoCode : public tRegion {
  virtual entity::EntityType GetEntityType() const { return entity::kEntityTypeGeoCode; }

  SERIALIZE_VIRTUAL(DEFAULT_CUSTOM / name*1 / lat*3 / lon*4);
};

}  // namespace region_data


#endif  // _UTIL_REGION_DATA_GEO_CODE_H_
