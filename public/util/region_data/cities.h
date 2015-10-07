// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

// This file defines all the data that is stored for each city.

#ifndef _UTIL_REGION_DATA_CITIES_H_
#define _UTIL_REGION_DATA_CITIES_H_

#include <deque>
#include <functional>
#include <memory>

#include "base/defs.h"
#include "util/file/csvreader.h"
#include "util/index/index.h"
#include "util/i18n/config.h"
#include "util/region_data/region.h"

extern string gFlag_em_cities_id;

namespace region_data {

struct tCity : public tRegion {
  // Returns the entity type for the given entity.
  virtual entity::EntityType GetEntityType() const { return entity::kEntityTypeCity; }

  void MergeFrom(const tCity* from) {
    tRegion::MergeFrom(from);
    const bool from_has_higher_priority = this->priority < from->priority;

    if ((state_iso_code_short.empty() || from_has_higher_priority)
        && !from->state_iso_code_short.empty()) {
      state_iso_code_short = from->state_iso_code_short;
    }
    if ((country_iso_code_2char.empty() || from_has_higher_priority)
        && !from->country_iso_code_2char.empty()) {
      country_iso_code_2char = from->country_iso_code_2char;
    }

    if ((metro_code.empty() || from_has_higher_priority)
        && !from->metro_code.empty()) {
      metro_code = from->metro_code;
    }
  }

  // The metro code for the city.
  string metro_code;

  // The ISO short code for the state.
  string state_iso_code_short;

  // The ISO 2 char country code.
  string country_iso_code_2char;

  CSV(eid | metro_code | name | state_iso_code_short | country_iso_code_2char |
      alternate_names | lat | lon | score);

  // This is only for testing. This struct should never be serialized in
  // production code. Use tStateReply instead.
  SERIALIZE(eid*9 / metro_code*1 / name*2 / state_iso_code_short*3 /
            country_iso_code_2char*4 / alternate_names*5 / lat*6 /
            lon*7 / score*8);
};

// The city data returned as RPC Reply.
struct tCityReply : public tRegionReply {
  // The state info.
  string state_code;
  string state_name;

  // The country info.
  string country_code;  // This is the ISO 2 char Code.
  string country_name;

  SERIALIZE(eid*1 / name*2 / latitude*3 / longitude*4 / state_code*5 / state_name*6 /
            country_code*7 / country_name*8);
};

// Returns the state RPC reply after converting the city storage data.
struct city_storage_to_reply :
    public region_storage_to_reply<tCity, tCityReply> {
  typedef region_storage_to_reply<tCity, tCityReply> super;

  virtual tCityReply operator() (const tCity& city) const;

  virtual tCityReply operator() (const tCity* city) const {
    return operator()(*city);
  }
};

struct tCityStateCountry {
  // All codes are ISO.
  string city_code, city_name;
  string state_code, state_name;
  string country_code, country_name;
  double latitude, longitude;
  string common_name;

  inline void MakeCommonName() {
    // city, city (foreign country)
    common_name = city_name;
    if (!city_code.empty())
      common_name += (", " + city_code);
    if (country_code != "US")
      common_name += (" (" + country_name + ")");
  }
  SERIALIZE(city_code*1 / city_name*2 / state_code*3 / state_name*4 / country_code*5 /
            country_name*6 / latitude*7 / longitude*8 / common_name*9);
};

struct order_cities_by_name : binary_function <tCity, tCity, bool> {
  // First sort by pre-defined state priority list, then sort by name.
  bool operator() (const tCity& c1, const tCity& c2) const {
    int p1 = I18N::ListPriority(c1.country_iso_code_2char);
    int p2 = I18N::ListPriority(c2.country_iso_code_2char);

    // Order by priority.
    if (p1 != p2) return p1 < p2;

    // Order by city name.
    if (c1.name != c2.name) return c1.name < c2.name;

    // Order cities with same name by state.
    if (c1.state_iso_code_short != c2.state_iso_code_short)
      return c1.state_iso_code_short < c2.state_iso_code_short;

    // Order cities with same name, state by country.
    return c1.country_iso_code_2char < c2.country_iso_code_2char;
  }

  bool operator() (const tCity* c1, const tCity* c2) const {
    return operator()(*c1, *c2);
  }

  bool operator() (const shared_ptr<tCity>& c1,
                   const shared_ptr<tCity>& c2) const {
    return operator()(*c1, *c2);
  }
};

struct order_cities_by_normalized_name : binary_function <tCity, tCity, bool> {
  // First sort by pre-defined state priority list, then sort by name.
  bool operator() (const tCity& c1, const tCity& c2) const {
    int p1 = I18N::ListPriority(c1.country_iso_code_2char);
    int p2 = I18N::ListPriority(c2.country_iso_code_2char);

    // Order by priority.
    if (p1 != p2) return p1 < p2;

    // Order by normalized city name.
    if (c1.normalized_name != c2.normalized_name) {
      return c1.normalized_name < c2.normalized_name;
    }

    // Order cities with same name by state.
    if (c1.state_iso_code_short != c2.state_iso_code_short)
      return c1.state_iso_code_short < c2.state_iso_code_short;

    // Order cities with same name, state by country.
    return c1.country_iso_code_2char < c2.country_iso_code_2char;
  }

  bool operator() (const tCity* c1, const tCity* c2) const {
    return operator()(*c1, *c2);
  }

  bool operator() (const shared_ptr<tCity>& c1,
                   const shared_ptr<tCity>& c2) const {
    return operator()(*c1, *c2);
  }
};

// Order cities by score.
struct order_cities_by_score : binary_function <tCity, tCity, bool> {
  bool operator() (const tCity& c1, const tCity& c2) const {
    if (c1.score != c2.score) return c1.score > c2.score;
    return order_name(c1, c2);
  }

  bool operator() (const tCity* c1, const tCity* c2) const {
    return operator()(*c1, *c2);
  }

  bool operator() (const shared_ptr<tCity>& c1,
                   const shared_ptr<tCity>& c2) const {
    return operator()(*c1, *c2);
  }

 private:
  order_cities_by_name order_name;
};

// This class provides an interface to access the cities data.
// Since this data is read only, there is only a single instance of the data.
class Cities : public Region<tCity, order_cities_by_score> {
  typedef Region<tCity, order_cities_by_score> super;

 public:
  virtual ~Cities() {}

  // Initialize the class.
  virtual bool Initialize();

  static Cities& Instance() {  // singleton instance
    struct Creator {
      Cities* CreateCities() {
        LOG(INFO) << "Creating Cities using Id: " << gFlag_em_cities_id;
        mutable_shared_proxy proxy = make_shared(gFlag_em_cities_id);
        ASSERT_NOTNULL(proxy);
        pin(proxy);
        return dynamic_cast<Cities*>(proxy.get());
      }
    };
    static Cities* the_one = Creator().CreateCities();
    return *the_one;
  }

  // Returns the city that matches the given city metro code.
  virtual const tCity* LookupByCode(const string& code) const;

  // Returns the city matching the given name. We expect the state and the
  // country code to be in ISO format.
  virtual const tCity* LookupByNameAndCode(const string& name, const string& state_code,
                                           const string& country_code) const;

  // Tries its best to normalize and return the city or cities matching the
  // given city name, state and country.
  // - If country is specified and not found, returns empty.
  // - If country is not specified returns all cities matching city name.
  // - If country is found and state is not found or is not specified, returns
  //   all matching cities within the country.
  // If multiple items are returned, end user may choose to use other means
  // (e.g. lat / long) to disambiguate.
  virtual vector<const tCity*> NormalizeAndLookup(const string& city, const string& state,
                                                  const string& country) const;

  // Returns the city name for the given city code.
  string GetCityName(const string& code) const {
   const tCity* city = LookupByCode(code);
   if (city) return city->name;

   return "";
  }

  static bool GetCityStateCountryInfo(const tCity* city, tCityStateCountry* result);

  virtual void Dedup(vector<const tCity*>* result) const;

 protected:
  // Declare this class as friend so that it call its constructor.
  friend class InitializeConfigureConstructor<Cities, string>;

  Cities() {}

  // Builds the indices for the cities.
  void BuildIndices();

  // Creates the primary indices for a new city.
  void PrimaryIndicesForCity(const tCity* city);

  // Indexes the city by the given keyword. Optionally generates prefix index
  // for the key if requested starting at the offset.
  // No prefix index is generated if the offset is string::npos.
  void IndexCityByKey(const tCity* city, const string& key);

 private:
  // Index cities by code.
  Index<const char*, const tCity*> city_metro_code_index_;
};

const tCity *FindCity(const string& city_name,
                      const string& state_code,
                      const string& country_code,
                      double lat, double lon,
                      double max_distance_error);

}  // namespace region_data

#endif
