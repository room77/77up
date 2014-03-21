// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// This file provides the basic interface/implementation for all
// region/geographical elements.

#ifndef _UTIL_REGION_DATA_REGION_H_
#define _UTIL_REGION_DATA_REGION_H_

#include <algorithm>
#include <deque>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "base/common.h"
#include "util/string/strutil.h"
#include "util/abbreviations/abbreviations.h"
#include "util/file/csvreader.h"
#include "util/entity/entity.h"
#include "util/entity/entity_manager.h"
#include "util/index/index.h"
#include "util/geo/latlong.h"
#include "util/region_data/utils.h"
#include "util/templates/comparator.h"

extern int gFlag_region_lat_grid_size;
extern int gFlag_region_long_grid_size;
extern int gFlag_region_latlong_index_max_items_per_key;
extern int gFlag_region_latlong_max_res;
extern int gFlag_region_name_index_max_items_per_key;

namespace region_data {

// The base class for all region elements.
struct tRegion : public entity::tEntity {
  virtual ~tRegion() {}

  double get_latitude() const { return lat; }
  double get_longitude() const { return lon; }

  bool has_lat_long() const {
    return !(get_latitude() == 0 && get_longitude() == 0);
  }

  template<typename T>
  int get_alternate_names(T* names) const {
    strutil::SplitString(alternate_names, "|", names);
    return names->size();
  }

  void MergeFrom(const tRegion* from) {
    const bool from_has_higher_priority = this->priority < from->priority;
    string new_normalized_alternate_name;
    if ((name.empty() || from_has_higher_priority) && !from->name.empty()) {
      // we are overwriting this name with from name; keep this name as
      // an alternate name
      new_normalized_alternate_name = normalized_name;
      name = from->name;
      normalized_name = from->normalized_name;
    } else {
      // we are keeping this name; keep from name as an alternate name
      new_normalized_alternate_name = from->normalized_name;
    }

    if ((!has_lat_long() || from_has_higher_priority) && from->has_lat_long()) {
      lat = from->get_latitude();
      lon = from->get_longitude();
    }

    // Fix alternate names.
    this->MergeAlternateNamesFrom(from, new_normalized_alternate_name);
    this->priority = max(this->priority, from->priority);
  }

  // @param strict - if false, the resulting alternate_names could have
  // duplicates and be out of order, which would need to be fixed before
  // writing to a file
  void AddAlternateName(const string& alternate_name, bool strict = true) {
    if (alternate_name.empty()) {
      ASSERT_DEV(false) << "empty alternate name";
      return;
    }
    if (strict) {
      set<string> alternate_names_set;
      get_alternate_names(&alternate_names_set);
      alternate_names_set.insert(alternate_name);
      alternate_names_set.erase(utils::NormalizeString(this->name));
      this->alternate_names = strutil::JoinString(alternate_names_set, "|");
    } else {
      if (!this->alternate_names.empty()) this->alternate_names += "|";
      this->alternate_names += alternate_name;
    }
  }

  // Dummy default implementation.
  virtual void SetEntityId() {}

  // The name of the region. This name is typically the ISO name.
  string name;

  // '|' separated list of alternate names for the region.
  // These names comprise non-standard/colloquial names for the region.
  string alternate_names;

  // The latitude and longitude for region.
  double lat = 0, lon = 0;

  double score = 0;

  // greater priority wins; e.g., priority == 1 wins priority == 0
  int priority = 0;

  // NOTE must be updated if the name changes
  string normalized_name;

  CSV(name | alternate_names | lat | lon);

  SERIALIZE_VIRTUAL(DEFAULT_CUSTOM / eid*1 / name*2 / alternate_names*3 / lat*4 / lon*5);

 private:
  void MergeAlternateNamesFrom(const tRegion *from,
                               const string& new_normalized_alternate_name) {
    // Need a use a set to make it deterministic.
    set<string> alternate_names_set;
    get_alternate_names(&alternate_names_set);
    from->get_alternate_names(&alternate_names_set);
    if (!new_normalized_alternate_name.empty()) {
      alternate_names_set.insert(new_normalized_alternate_name);
    }
    alternate_names_set.erase(utils::NormalizeString(this->name));
    this->alternate_names = strutil::JoinString(alternate_names_set, "|");
  }
};

// Region data returned as RPC reply.
struct tRegionReply {
  virtual ~tRegionReply() {}

  // The eid for the reply.
  string eid;

  // The name of the region.
  string name;

  // The latitude and longitude for the region.
  double latitude = 0, longitude = 0;

  // We do not serialize alternate names for now.
  SERIALIZE(eid*1 / name*2 / latitude*3 / longitude*4);
};

// Returns the region RPC reply after converting the region storage data.
template<typename Storage=tRegion, typename Reply=tRegionReply>
struct region_storage_to_reply {
  virtual ~region_storage_to_reply() {}

  virtual Reply operator() (const Storage& region) const {
    Reply reply;
    reply.eid = region.GetEid();
    reply.name = region.name;
    reply.latitude = region.get_latitude();
    reply.longitude = region.get_longitude();
    return reply;
  }

  virtual Reply operator() (const Storage* region) const {
    return operator()(*region);
  }
};

template<typename Data=tRegion>
struct order_region_by_name : binary_function <Data, Data, bool> {
  bool operator() (const Data& r1, const Data& r2) const {
    return r1.name < r2.name;
  }

  bool operator() (const Data* r1, const Data* r2) const {
    return operator ()(*r1, *r2);
  }
};

// Returns a new object having the same data as the object pointed to by the
// input pointer.
template<typename Data=tRegion>
struct ptr_to_data {
  Data operator() (const Data* ptr) const {
    Data tmp = *ptr;
    return tmp;
  }
};

// Base class for all geographic elements.
template <typename Data=tRegion, typename Comparator=order_region_by_name<Data>>
class Region : public entity::EntityManager {
 public:
  typedef Comparator CompType;
  typedef Data DataType;

  virtual ~Region() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  typedef deque<shared_ptr<Data>> tStorage;

  // Returns the region for the given entity id.
  // The default implementation return nullptr.
  // Derived classes must build and parse
  virtual const Data* LookupUniqueByEntityId(const string& entity_id) const;

  // Generates a list of regions matching the given name.
  // The return value is the size of the matched regions.
  virtual int LookupByName(const string& name, vector<const Data*>* result,
                           int max_results = std::numeric_limits<int>::max(),
                           const Comparator& comp = Comparator()) const;

  // If the name is unique, returns the matching region.
  // NOTE: Do not use this function if the name index is not unique, use the
  // function above instead.
  virtual const Data* LookupUniqueByName(const string& name) const;

  // Generates a list of regions close to a given latlong.
  // The return value is the size of the matched regions.
  // Only closest 'max_results' are shown that lie within 'max_radius_km'
  // distance from the reference point within the neighboring grids.
  // Note: Setting 'max_results = inf' and 'max_radius_km = inf' will not return
  // all possible regions but only those within a grid index distance from the
  // reference point.
  // The returned vector is a pair <Data*, 'distance from reference point'>
  // sorted in increasing order of distance.
  virtual int LookupByLatLong(const LatLong& reference_point,
      vector<pair<const Data*, double> >* result,
      int max_results = std::numeric_limits<int>::max(),
      double max_radius_km = std::numeric_limits<double>::max()) const;

  // find the closest region by lat long
  // may return nullptr
  virtual const Data * FindByLatLong(const LatLong& reference_point) const {
    vector<pair<const Data *, double> > result;
    if (!LookupByLatLong(reference_point, &result, 1)) {
      return nullptr;
    }
    return result[0].first;
  }

  // Parses the input as best as possible and returns all possible matches.
  virtual int Parse(const string& input,
                    vector<const Data*>* result,
                    int max_results = std::numeric_limits<int>::max(),
                    const Comparator& comp = Comparator()) const;

  template<typename Res, typename Converter=ptr_to_data<Data> >
  int ParseAndConvert(const string& input, vector<Res>* result,
                      int max_results = std::numeric_limits<int>::max(),
                      const Converter& conv = Converter(),
                      const Comparator& comp = Comparator()) const;

  // Returns the complete storage in default sorted order.
  const tStorage& storage() const { return storage_; }

  // Returns the first n items in storage.
  int GetFirstN(int n, vector<const Data*>* result) const;

  // Dedups a set of *already sorted* list of results.
  virtual void Dedup(vector<const Data*>* result) const;

  // Sorts the results.
  virtual void SortResults(vector<const Data*>* result, int max_results,
                           const Comparator& comp) const;

  // Latlong to grid ID.
  int LatitudeGridID(const LatLong& p) const {
    return static_cast<int>(p.get_multiplied_latitude() / params_.lat_grid_size);
  }

  int LongitudeGridID(const LatLong& p) const {
    return static_cast<int>(p.get_multiplied_longitude() / params_.long_grid_size);
  }

  // Returns the nearest region and its distance to a reference point from among the given
  // candidates.
  static pair<const Data*, double> GetNearestTo(const LatLong& reference,
                                                const vector<const Data*>& candidates);

 protected:
  struct ConfigParams {
    string file;
    bool create_entity_ids = false;
    bool build_entity_id_index = false;
    bool build_name_index = false;
    bool name_index_is_unique = false;
    bool build_latlong_index = false;
    int name_index_max_items_per_key = gFlag_region_name_index_max_items_per_key;
    int latlong_index_max_items_per_key = gFlag_region_latlong_index_max_items_per_key;
    int lat_grid_size = gFlag_region_lat_grid_size;
    int long_grid_size = gFlag_region_long_grid_size;
  };

  virtual bool Initialize(const ConfigParams& params);

  // Initializes the storage.
  virtual int InitStorage(const string& file);

  // Builds the ids if necessary.
  void CreateEnitityIds();

  // Builds the entityid index. If reset is true, first clears the current index
  // and rebuilds. Otherwise simply adds to the index.
  void BuildEntityIdIndex(bool reset = true);

  // Builds the name index. If reset is true, first clears the current index
  // and rebuilds. Otherwise simply adds to the index.
  void BuildNameIndex(bool reset = true);

  // Build the LatLong index. If reset is true, first clears the current index
  // and rebuilds. Otherwise simply adds to the index.
  void BuildLatLongIndex(bool reset = true);

  // Storage for the the data.
  tStorage storage_;

  // Index by entity_id.
  Index<const char*, const Data*> entity_id_index_;

  // Index by name.
  Index<const char*, const Data*> name_index_;

  // Index by Latlong.
  Index<pair<int, int>, const Data*> latlong_index_;

  // The configuration params.
  ConfigParams params_;
};

template <typename Data, typename Comparator>
bool Region<Data, Comparator>::Initialize(
    const Region<Data, Comparator>::ConfigParams& params) {
  params_ = params;
  if (!InitStorage(params_.file)) return false;

  name_index_.set_max_items_per_key(params_.name_index_max_items_per_key);
  latlong_index_.set_max_items_per_key(params_.latlong_index_max_items_per_key);

  if (params.create_entity_ids) CreateEnitityIds();
  if (params.build_entity_id_index) BuildEntityIdIndex();
  if (params.build_name_index) BuildNameIndex();
  if (params.build_latlong_index) BuildLatLongIndex();

  return true;
}

template <typename Data, typename Comparator>
int Region<Data, Comparator>::InitStorage(const string& file) {
  ASSERT(!file.empty());
  CSV::CSVReader<tStorage, Data, CSV::PointerContainerInserter<tStorage, Data> > reader(file, ',');
  ASSERT(reader.Read(&storage_)) << "Could not read region data from: " << file;
  return storage_.size();
}

template <typename Data, typename Comparator>
void Region<Data, Comparator>::CreateEnitityIds() {
  for (shared_ptr<Data>&  data : storage_) data->SetEntityId();
}

template <typename Data, typename Comparator>
void Region<Data, Comparator>::BuildEntityIdIndex(bool reset) {
  if (reset) entity_id_index_.Clear();

  // Index by eid.
  for (const shared_ptr<Data>&  data : storage_)
    entity_id_index_.AddToIndex(data->eid, data.get(), true);
}

template <typename Data, typename Comparator>
void Region<Data, Comparator>::BuildNameIndex(bool reset) {
  if (reset) name_index_.Clear();

  // Note: We do these in different loops to maintain order as only top n are
  // kept for every key.

  // Index by name.
  for (const shared_ptr<Data>&  data : storage_) {
    name_index_.AddToIndex(utils::NormalizeString(data->name), data.get(),
                           params_.name_index_is_unique);
  }

  // Index by alternate names.
  for (const shared_ptr<Data>&  data : storage_) {
    unordered_set<string> alternate_names;
    if (!data->get_alternate_names(&alternate_names)) continue;
    for (const string& name : alternate_names) {
      name_index_.AddToIndex(utils::NormalizeString(name), data.get(),
                             params_.name_index_is_unique);
    }
  }
}

template <typename Data, typename Comparator>
void Region<Data, Comparator>::BuildLatLongIndex(bool reset) {
  if (reset) latlong_index_.Clear();

  // Index by alternate names.
  for (const shared_ptr<Data>& data : storage_) {
    LatLong ll = LatLong::Create(data->get_latitude(), data->get_longitude());
    latlong_index_.AddToIndex(make_pair(LatitudeGridID(ll), LongitudeGridID(ll)),
                              data.get(), false);
  }
}

template <typename Data, typename Comparator>
const Data* Region<Data, Comparator>::LookupUniqueByEntityId(
    const string& entity_id) const {
  if (entity_id.empty()) return nullptr;
  return entity_id_index_.RetrieveUnique(entity_id);
}

template <typename Data, typename Comparator>
int Region<Data, Comparator>::LookupByName(const string& name,
    vector<const Data*>* result, int max_results, const Comparator& comp) const {
  const string s = utils::NormalizeString(name);
  if (s.empty()) return 0;

  name_index_.Retrieve(s, result);

  // Do the abbreviated name lookup.
  static util::abbr::Abbreviation::shared_proxy abbr =
      util::abbr::Abbreviation::make_shared("name_abbr");
  ASSERT_NOTNULL(abbr);
  // NOTE: After 032fa75, we keep abbreviations instead of expanded versions to reduce space.
  // This works for cities. All other data sources should also do the same if they want
  // abbreviation matching.
  string abbreviated_s = abbr->ReplaceAllCompletions(s);
  if (s != abbreviated_s) {
    // There is an abbreviation possible for the string. Lets do the extra lookup.
    vector<const Data*> abbreviated_result;
    name_index_.Retrieve(abbreviated_s, &abbreviated_result);
    if (abbreviated_result.size()) {
      result->reserve(result->size() + abbreviated_result.size());
      std::move(abbreviated_result.begin(), abbreviated_result.end(), std::back_inserter(*result));
    }
  }

  if (result->size()) {
    // Sort the matches by input comparator.
    SortResults(result, max_results, comp);
    // Dedup the results.
    Dedup(result);
  }
  if (params_.name_index_is_unique)
    ASSERT_GE(1, result->size()) << "Multiple regions for name" << name;

  return result->size();
}

template <typename Data, typename Comparator>
const Data* Region<Data, Comparator>::LookupUniqueByName(const string& name) const {
  ASSERT(params_.name_index_is_unique);
  vector<const Data*> res;
  LookupByName(name, &res, 1);
  return res.size() ? res[0] : nullptr;
}

template <typename Data, typename Comparator>
int Region<Data, Comparator>::LookupByLatLong(const LatLong& reference_point,
    vector<pair<const Data*, double>>* result, int max_results,
    double max_radius_km) const {
  int latitude_grid_id = LatitudeGridID(reference_point);
  int longitude_grid_id = LongitudeGridID(reference_point);
  // retrieve from 9 nearby grid cells.
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      vector<const Data*> nearby;
      latlong_index_.Retrieve(
          make_pair(latitude_grid_id + i, longitude_grid_id + j), &nearby);

      for (const Data* data : nearby) {
        LatLong loc = LatLong::Create(data->get_latitude(), data->get_longitude());
        if (abs(loc.get_multiplied_latitude() - reference_point.get_multiplied_latitude())
            < params_.lat_grid_size &&
            abs(loc.get_multiplied_longitude() - reference_point.get_multiplied_longitude())
            < params_.long_grid_size) {
          // Calculate surface distance.
          double distance = reference_point.SurfaceDistanceFrom(loc);
          if (distance <= max_radius_km) {
            result->push_back(make_pair(data, distance));
          }
        }
      }
    }
  }

  // Sort the matches by distance.
  if (result->size()) {
    partial_sort(result->begin(),
                 result->begin() + min<int>(max_results, result->size()),
                 result->end(),
                 util::tl::less_second<pair<const Data*, double> >());

    if (result->size() > max_results)
      result->resize(max_results);

    vector<const Data*> dup_candidates;
    dup_candidates.reserve(result->size());
    for (const auto& pair : *result) dup_candidates.push_back(pair.first);
    // Dedup the results.
    Dedup(&dup_candidates);
    if (dup_candidates.size() < result->size()) {
      int i = 0;
      auto iter = remove_if(result->begin(), result->end(),
                            [&dup_candidates, &i](const pair<const Data*, double>& pair) -> bool {
                               if (i < dup_candidates.size() && dup_candidates[i] == pair.first) {
                                 ++i;
                                 return false;
                               }
                               return true;
                            });
      if (iter != result->end()) result->resize(iter - result->begin());
    }
  }
  return result->size();
}

template <typename Data, typename Comparator>
int Region<Data, Comparator>::Parse(const string& input,
    vector<const Data*>* result, int input_max_results,
    const Comparator& comp) const {
  if (input.empty() || input_max_results == 0) return 0;

  // Do a lookup by name.
  return LookupByName(input, result, input_max_results, comp);
}

template <typename Data, typename Comparator>
template <typename Res, typename Converter>
int Region<Data, Comparator>::ParseAndConvert(const string& input, vector<Res>* result,
    int max_results, const Converter& conv, const Comparator& comp) const {
  vector<const Data*> res;
  if (!Parse(input, &res, max_results, comp)) return 0;

  result->reserve(res.size());
  for (const Data* data : res) {
    result->push_back(conv(data));
  }
  return result->size();
}

template <typename Data, typename Comparator>
int Region<Data, Comparator>::GetFirstN(int n, vector<const Data*>* result) const {
  if (n < 1) return 0;

  result->clear();
  result->reserve(n);
  for (int i = 0; i < n && i < storage_.size(); ++i)
    result->push_back(storage_[i].get());

  return result->size();
}

template <typename Data, typename Comparator>
void Region<Data, Comparator>::Dedup(vector<const Data*>* result) const {
  unordered_set<const Data*> seen;
  auto iter = remove_if(result->begin(), result->end(),
                        [&seen](const Data* data) -> bool {
                             if (seen.find(data) != seen.end()) return true;
                             seen.insert(data); return false;
                        });

  if (iter != result->end()) result->resize(iter - result->begin());
}

template <typename Data, typename Comparator>
void Region<Data, Comparator>::SortResults(vector<const Data*>* result,
    int max_results, const Comparator& comp) const {
  if (result->empty()) return;

  // Sort the matches by input comparator.
  partial_sort(result->begin(),
               result->begin() + min<int>(max_results, result->size()),
               result->end(), comp);

  if (result->size() > max_results) result->resize(max_results);
}

template <typename Data, typename Comparator>
pair<const Data*, double> Region<Data, Comparator>::GetNearestTo(const LatLong& reference,
    const vector<const Data*>& candidates) {
  pair<const Data*, double> nearest(nullptr, numeric_limits<double>::max());
  for (const Data* candidate : candidates) {
    double distance = reference.SurfaceDistanceFrom(LatLong::Create(candidate->lat, candidate->lon));
    if (distance < nearest.second) nearest = {candidate, distance};
  }
  return nearest;
}

}  // namespace region_data


#endif  // _UTIL_REGION_DATA_REGION_H_
