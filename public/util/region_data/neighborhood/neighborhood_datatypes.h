// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _UTIL_REGION_DATA_NEIGHBORHOOD_NEIGHBORHOOD_DATATYPES_H_
#define _UTIL_REGION_DATA_NEIGHBORHOOD_NEIGHBORHOOD_DATATYPES_H_

#include "base/defs.h"
#include "util/entity/entity_id.h"
#include "util/region_data/region.h"
#include "util/serial/serializer.h"

namespace region_data {

// Maponics loaction info.
struct tNeighborhoodInfo {
  string id = "-1";
  string name;
  string descriptor;
  string country_code = "US";
  // state is not guaranteed for things other than city and above, it is
  // just used for creating seo pages
  string state;
  // mcd is only for neighborhoods. in ALL cases, this is the name of
  // of the city
  string mcd;
  int zoom_level = -1;
  int hotel_count = 0;
  vector<float> center;
  vector<string> parents;
  vector<string> children;

  SERIALIZE(DEFAULT_CUSTOM / id*1 / name*2 / state*3 / mcd*4 / zoom_level*5 /
            hotel_count*6 / center*7 / parents*8 / children*9 /
            country_code*10);
};

// The extended location info with bounding coordinates.
struct tNeighborhoodInfoWithBounds : public tRegion {
  // Returns the entity type for the given entity.
  virtual entity::EntityType GetEntityType() const { return entity::kEntityTypeNeighborhood; }

  virtual void SetEntityId() {
    SetEid(::entity::GetEntityIdFromBaseId(entity::kEntityTypeNeighborhood, loc.id));

    // Though this is not the best place for this, we use this function to fix the latitudes and
    // longitudes as this is called at the perfect time for fixing up the data structure.
    lat = loc.center[1];
    lon = loc.center[0];
  }

  tNeighborhoodInfo loc;
  vector<vector<vector<float>>> coords;

  SERIALIZE(DEFAULT_CUSTOM / loc*1 / coords*2);
};

// The neighborhood data returned as RPC Reply.
struct tNeighborhoodInfoReply : public tRegionReply {
  string id = "-1";

  // The city info.
  string mcd;
  string city;

  // The state info.
  string state;

  int zoom_level = -1;
  int hotel_count = 0;

  SERIALIZE(eid*9 / name*1 / id*2 / latitude*3 / longitude*4 / mcd*5 / state*6 / zoom_level*7 /
            hotel_count*8);
};

}  // namespace region_data


#endif  // _UTIL_REGION_DATA_NEIGHBORHOOD_NEIGHBORHOOD_DATATYPES_H_
