// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

// This file defines all the data that is stored for each attraction.

#ifndef _UTIL_REGION_DATA_ATTRACTIONS_H_
#define _UTIL_REGION_DATA_ATTRACTIONS_H_

#include <deque>
#include <functional>
#include <memory>

#include "base/common.h"
#include "util/i18n/config.h"
#include "util/region_data/region.h"

extern string gFlag_em_attractions_id;

namespace region_data {

struct tAttraction : public tRegion {
  // Returns the entity type for the given entity.
  virtual entity::EntityType GetEntityType() const { return entity::kEntityTypeAttraction; }

  // The city name.
  string city;

  // The entity id of the city.
  string city_eid;

  // The ISO short code for the state.
  string state_iso_code_short;

  // The ISO 2 char country code.
  string country_iso_code_2char;

  // The classification of attraction.
  string classification;

  CSV(eid | name | city_eid | city | state_iso_code_short | country_iso_code_2char |
      alternate_names | lat | lon | classification);

  // This is only for testing. This struct should never be serialized in
  // production code. Use tAttractionReply instead.
  SERIALIZE(eid*8 / name*1 / lat*2 / lon*3 / city*4 /
            state_iso_code_short*5 / country_iso_code_2char*6 / classification*7 /
            city_eid*9);
};

// The attraction data returned as RPC Reply.
struct tAttractionReply : public tRegionReply {
  // The city info.
  string city;

  // The state info.
  string state_code;
  string state_name;

  // The country info.
  string country_code;
  string country_name;

  // The classification of attraction.
  string classification;

  SERIALIZE(eid*10 / name*1 / latitude*2 / longitude*3 / city*4 / state_code*5 / state_name*6 /
            country_code*7 / country_name*8 / classification*9);
};

// Returns the attraction RPC reply after converting the attraction storage data.
struct attraction_storage_to_reply :
    public region_storage_to_reply<tAttraction, tAttractionReply> {
  typedef region_storage_to_reply<tAttraction, tAttractionReply> super;

  virtual tAttractionReply operator() (const tAttraction& attr) const;

  virtual tAttractionReply operator() (const tAttraction* attr) const {
    return operator()(*attr);
  }
};

struct order_attractions_by_name :
    binary_function <tAttraction, tAttraction, bool> {
  // First sort by pre-defined state priority list, then sort by name.
  bool operator() (const tAttraction& a1, const tAttraction& a2) const;

  bool operator() (const tAttraction* a1, const tAttraction* a2) const {
    return operator ()(*a1, *a2);
  }
};

// This class provides an interface to access the attractions data.
// Since this data is read only, there is only a single instance of the data.
class Attractions : public Region<tAttraction, order_attractions_by_name> {
  typedef Region<tAttraction, order_attractions_by_name> super;

 public:
  virtual ~Attractions() {}

  // Initialize the class.
  virtual bool Initialize();

  static Attractions& Instance() {  // singleton instance
    struct Creator {
      Attractions* CreateAttractions() {
        LOG(INFO) << "Creating Attractions using Id: " << gFlag_em_attractions_id;
        mutable_shared_proxy proxy = make_shared(gFlag_em_attractions_id);
        ASSERT_NOTNULL(proxy);
        pin(proxy);
        return dynamic_cast<Attractions*>(proxy.get());
      }
    };
    static Attractions* the_one = Creator().CreateAttractions();
    return *the_one;
  }

  // Generates a list of attractions matching the given name city,
  // state_code and country_code. The return value is the size of the matched
  // attractions.
  virtual int LookupByNameEx(const string& name, const string& city,
                             const string& state_code, const string& country_code,
                             vector<const tAttraction*>* result,
                             int max_results = std::numeric_limits<int>::max()) const;

 protected:
  // Declare this class as friend so that it call its constructor.
  friend class InitializeConfigureConstructor<Attractions, string>;

  Attractions() {}
};

}  // namespace region_data

#endif  // _UTIL_REGION_DATA_ATTRACTIONS_H_
