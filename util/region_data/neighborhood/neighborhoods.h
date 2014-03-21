// Copyright 2013 Room77, Inc.
// Author: akearney@room77.com (Andy Kearney), pramodg@room77.com (Pramod Gupta)

// This file defines all the data that is stored for each neighborhood.

#ifndef _UTIL_REGION_DATA_NEIGHBORHOOD_NEIGHBORHOODS_H_
#define _UTIL_REGION_DATA_NEIGHBORHOOD_NEIGHBORHOODS_H_

#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <unordered_map>
#include <utility>

#include "base/defs.h"
#include "util/region_data/neighborhood/neighborhood_datatypes.h"
#include "util/region_data/region.h"
#include "util/templates/container_util.h"

extern string gFlag_em_neighborhoods_id;

namespace region_data {

// Returns the Neighborhood RPC reply after converting the neighborhood storage data.
struct neighborhood_storage_to_reply
    : public region_storage_to_reply<tNeighborhoodInfoWithBounds, tNeighborhoodInfoReply> {
  typedef region_storage_to_reply<tNeighborhoodInfoWithBounds, tNeighborhoodInfoReply> super;

  virtual tNeighborhoodInfoReply operator() (const tNeighborhoodInfoWithBounds& nbr) const;

  virtual tNeighborhoodInfoReply operator() (const tNeighborhoodInfoWithBounds* nbr) const {
    return operator()(*nbr);
  }
};

// Order neighborhoods by name.
struct order_neighborhoods_by_name
    : binary_function <tNeighborhoodInfoWithBounds, tNeighborhoodInfoWithBounds, bool> {
  // First sort by pre-defined state priority list, then sort by name.
  bool operator() (const tNeighborhoodInfoWithBounds& n1,
                   const tNeighborhoodInfoWithBounds& n2) const;

  bool operator() (const tNeighborhoodInfoWithBounds* n1,
                   const tNeighborhoodInfoWithBounds* n2) const {
    return operator ()(*n1, *n2);
  }

  bool operator() (const shared_ptr<tNeighborhoodInfoWithBounds>& n1,
                   const shared_ptr<tNeighborhoodInfoWithBounds>& n2) const {
    return operator()(*n1, *n2);
  }
};

// Sorts neighborhoods by the number of hotels in them (in descending order).
struct order_neighborhoods_by_hotels
    : binary_function <tNeighborhoodInfoWithBounds, tNeighborhoodInfoWithBounds, bool> {
  bool operator() (const tNeighborhoodInfoWithBounds& n1,
                   const tNeighborhoodInfoWithBounds& n2) const {
    if (n1.loc.hotel_count != n2.loc.hotel_count)
      return n1.loc.hotel_count > n2.loc.hotel_count;

    return order_name(n1, n2);
  }

  bool operator() (const tNeighborhoodInfoWithBounds* n1,
                   const tNeighborhoodInfoWithBounds* n2) const {
    return operator()(*n1, *n2);
  }

  bool operator() (const shared_ptr<tNeighborhoodInfoWithBounds>& n1,
                   const shared_ptr<tNeighborhoodInfoWithBounds>& n2) const {
    return operator()(*n1, *n2);
  }

  order_neighborhoods_by_name order_name;
};


// This class provides an interface to access the neighborhoods data.
// Since this data is read only, there is only a single instance of the data.
class Neighborhoods : public Region<tNeighborhoodInfoWithBounds, order_neighborhoods_by_hotels> {
  typedef Region<tNeighborhoodInfoWithBounds, order_neighborhoods_by_hotels> super;

 public:
  virtual ~Neighborhoods() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize();

  static Neighborhoods& Instance() {  // singleton instance
    struct Creator {
      Neighborhoods* CreateNeighborhoods() {
        LOG(INFO) << "Creating Neighborhoods using Id: " << gFlag_em_neighborhoods_id;
        mutable_shared_proxy proxy = make_shared(gFlag_em_neighborhoods_id);
        ASSERT_NOTNULL(proxy);
        pin(proxy);
        return dynamic_cast<Neighborhoods*>(proxy.get());
      }
    };
    static Neighborhoods* the_one = Creator().CreateNeighborhoods();
    return *the_one;
  }

  const tNeighborhoodInfoWithBounds* LookupUniqueByNeighborhoodId(const string& id) const {
    return ::util::tl::FindWithDefault(id_location_map_, id, nullptr);
  }

  // Returns all the neighborhoods associated with the hid.
  vector<const tNeighborhoodInfoWithBounds*> LookupByHid(int hid) const;

  int LookupByLatLong(const LatLong& ref_point,
      map<int, vector<pair<const tNeighborhoodInfoWithBounds*, double>>>* result,
      int max_results = std::numeric_limits<int>::max(),
      float max_radius_km = std::numeric_limits<float>::max(),
      int min_hotel_in_neighborhood = 0) const;

  void RetrieveNeighborhood(const string& name, const string& mcd,
      const string& state_code, vector<const tNeighborhoodInfoWithBounds*>* result,
      int max_results = std::numeric_limits<int>::max()) const;

  void RetrieveNeighborhood(const string& name, const string& mcd,
      const string& state_code, vector<tNeighborhoodInfoReply>* result,
      int max_results = std::numeric_limits<int>::max()) const;

  const map<int, int>& ZoomToIndexSize() const { return zoom_indexsize_map_; }

 protected:
  // Declare this class as friend so that it call its constructor.
  friend class InitializeConfigureConstructor<Neighborhoods, string>;

  Neighborhoods() = default;

  // Initializes the storage.
  virtual int InitStorage(const string& file);

  // Build all the different indices.
  void BuildIndices();

  // Map from hid to range of indices in the neighborhoods list which

 private:
  void BuildNameIndex();
  void BuildLatLongIndex();
  void BuildIdToLocationMap();
  void BuildZoomToIndexSizeMap();
  void BuildHidToLocationMap();

  // Map from neighborhood id to neighborhood info.
  unordered_map<string, const tNeighborhoodInfoWithBounds*> id_location_map_;

  // Map from zoom level to integer index size.
  map<int, int> zoom_indexsize_map_;

  // Map from zoom_level -> Index of latlong grid id -> neighborhood info.
  map<int, HeavyIndex<pair<int, int>, tNeighborhoodInfoWithBounds*>> neighborhood_latlong_indices_;

  // Map from hid to list of locations associated with it.
  // Note: This could simply have been implemented as unordered_map<hid, vector<neighborhoods>>.
  // For storage optimization, this is implemented in two parts.
  // 1. deque<tNeighborhoodInfoWithBounds*>
  // 2. unordered_map<int, NeighborhoodIndexRange>
  //    where NeighborhoodIndexRange defines the start and end of the neighborhoods relevant to the
  //    the hid in the deque.
  // TODO(oztekin, pramodg): Use a generic container to abstract this functionality.
  // Currently, we will make a copy and return a vector at run time. In the future we can return a
  // custom iterator on deque with a valid begin and end.
  unordered_map<int, pair<int, int>> hid_location_range_map_;  // hid -> pair<start, length>
  deque<const tNeighborhoodInfoWithBounds*> location_list_;
};

} // namespace region_data


#endif  // _UTIL_REGION_DATA_NEIGHBORHOOD_NEIGHBORHOODS_H_
