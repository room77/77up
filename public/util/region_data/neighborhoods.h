#ifndef _UTIL_REGION_DATA_NEIGHBORHOODS_H_
#define _UTIL_REGION_DATA_NEIGHBORHOODS_H_

#include "util/region_data/cities.h"
#include "util/region_data/neighborhood/neighborhood_datatypes.h"
#include "base/defs.h"

// tNeighborhood is (tentatively) defined here for compatibility across
// different tRegion derivatives. The definition mainly mimics that of
// tAttraction

namespace region_data {

struct tNeighborhood : public region_data::tRegion {
  tNeighborhood() = default;
  explicit tNeighborhood(const region_data::tNeighborhoodInfoWithBounds& n);

  // The city name.
  string city;

  // The entity id of the city.
  string city_eid;

  // The ISO short code for the state.
  string state_iso_code_short;

  // The ISO 2 char country code.
  string country_iso_code_2char;

  int zoom_level = -1;

  CSV(eid | name | city_eid | city | state_iso_code_short | country_iso_code_2char |
      alternate_names | lat | lon);

  // This is only for testing. This struct should never be serialized in
  // production code. Use tAttractionReply instead.
  SERIALIZE(eid*8 / name*1 / lat*2 / lon*3 / city*4 /
            state_iso_code_short*5 / country_iso_code_2char*6 /
            city_eid*9);
}; // struct tNeighborhood

namespace utils {

struct order_neighborhoods_by_name
    : binary_function <tNeighborhood, tNeighborhood, bool> {

  // First sort by pre-defined state priority list, then sort by name.
  bool operator()(const tNeighborhood& n1, const tNeighborhood& n2) const;

  bool operator()(const tNeighborhood *n1, const tNeighborhood *n2) const {
    return operator()(*n1, *n2);
  }
};

class Neighborhoods
    : public Region<tNeighborhood, order_neighborhoods_by_name> {

  using super = Region<tNeighborhood, order_neighborhoods_by_name>;

 public:
  static const Neighborhoods& Instance();

  virtual ~Neighborhoods() { }

  virtual bool Configure(const string& op) { return true; }

  virtual bool Initialize();

//  const vector<shared_ptr<tNeighborhood>>& storage() const { return storage_; }

  // TODO
//  const tNeighborhood *LookupUniqueByEntityId(const string& eid) const {
//    return ::util::tl::FindWithDefault(index_by_eid_, eid, nullptr);
//  }

 private:
  Neighborhoods() = default;
//  vector<shared_ptr<tNeighborhood>> storage_;
//  unordered_map<string, const tNeighborhood *> index_by_eid_;

  friend class InitializeConfigureConstructor<Neighborhoods, string>;
}; // class Neighborhoods

void NeighborhoodExample();

}  // namespace region_data::utils

const tCity *FindCity(const tNeighborhoodInfoWithBounds& neighborhood);

}  // namespace region_data

#endif
