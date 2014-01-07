// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Maintains the best n items in a streaming model.
// Note: This class is not thread safe.

#ifndef _PUBLIC_UTIL_TEMPLATES_CONTAINER_BEST_N_H_
#define _PUBLIC_UTIL_TEMPLATES_CONTAINER_BEST_N_H_

#include <functional>
#include <unordered_map>

#include "base/defs.h"
#include "base/logging.h"
#include "util/templates/container/ra_heap.h"
#include "util/templates/comparator.h"
#include "util/serial/serializer.h"

namespace tl {

using ::util::tl::compare_member;

enum {
  kBestNInsertStatusInvalid = 0,
  kBestNInsertStatusDidNotAddBegin = 0,
  kBestNInsertStatusDidNotAddFailure,
  kBestNInsertStatusDidNotAddNewWorse,
  kBestNInsertStatusDidNotAddExistingBetter,
  kBestNInsertStatusDidNotAddEnd,

  kBestNInsertStatusAddedBegin = 10,
  kBestNInsertStatusAddedExistingWorse,
  kBestNInsertStatusAddedNew,
  kBestNInsertStatusAddedEnd,
};
typedef uint16_t BestNInsertStatus;

template<typename T, typename Comparator = std::less<T>>
class BestN {
  // The internal item used to maintain the inserted data.
  struct tInternalItem {
    string key;
    T data;
    size_t index;
  };

 public:
  static bool ItemAdded(BestNInsertStatus status) {
    return kBestNInsertStatusAddedBegin < status && status < kBestNInsertStatusAddedEnd;
  }

  static bool NewItemAdded(BestNInsertStatus status) {
    return status == kBestNInsertStatusAddedNew;
  }

  static bool ItemIsKnown(BestNInsertStatus status) {
    return status == kBestNInsertStatusDidNotAddExistingBetter ||
        status == kBestNInsertStatusAddedExistingWorse;
  }

  explicit BestN(size_t capacity) : capacity_(capacity) {};

  // Returns the capacity for the container.
  size_t capacity() const { return capacity_; }

  // Returns the current size of the container.
  size_t size() const { return kv_map_.size(); }

  // Returns true if the container is empty.
  size_t empty() const { return kv_map_.empty(); }

  // Returns the worst item currently in the list.
  // This assumes size() > 0.
  const T& worst() const {
    tInternalItem* item = ra_heap_.top();
    return item->data;
  }

  // Inserts a new item associated with the key.
  BestNInsertStatus insert(const string& key, const T& item) {
    // Check if the item is already known, in that case, simply update the value.
    BestNInsertStatus status = CheckAndUpdateExistingItem(key, item);
    if (ItemIsKnown(status)) return status;

    // This is a new item.

    // If we are less than capacity, simply add this item.
    if (size() < capacity()) return AddNewItem(key, item);

    // If we are already at capacity, see if this item is better than the top item, if so
    // remove the top item and insert the new item.

    // Compare this item with the worst item, merely being equal is not enough.
    if (!comp_(item, worst())) return kBestNInsertStatusDidNotAddNewWorse;

    // Remove the top item.
    RemoveTopItem();
    // Add the new item.
    status = AddNewItem(key, item);
    return status;
  }

  vector<T> GetBestN() {
    // Check if we need to rebuild the heap.
    if (ra_heap_.size() < kv_map_.size()) RebuildHeap();

    vector<T> res;
    res.resize(ra_heap_.size());
    for (int i = ra_heap_.size() - 1; i >= 0; --i)
      res[i] = ra_heap_.pop()->data;

    return res;
  }

  // Serialization methods. We only support to*,
  // Currently there is no need to support from* and is not possible to add it either as we
  // loose the key associated with the item. If we really need to do this, we can change the
  // API to accept a hasher for T and generate the key from there rather than asking for a
  // key explicitly.
  bool FromBinary(istream& in,
      const serial::BinaryDeSerializationParams& params = serial::BinaryDeSerializationParams()) {
    ASSERT(false);
    return false;
  }

  void ToBinary(ostream& out,
      const serial::BinarySerializationParams& params = serial::BinarySerializationParams()) const {
    vector<T> res = const_cast<BestN*>(this)->GetBestN();
    serial::Serializer::ToBinary(out, res, params);
  }

  bool FroJSON(istream& in,
      const serial::JSONDeSerializationParams& params = serial::JSONDeSerializationParams()) {
    ASSERT(false);
    return false;
  }

  void ToJSON(ostream& out,
      const serial::JSONSerializationParams& params = serial::JSONSerializationParams()) const {
    vector<T> res = const_cast<BestN*>(this)->GetBestN();
    serial::Serializer::ToJSON(out, res, params);
  }

 private:
  BestNInsertStatus AddNewItem(const string& key, const T& item) {
    auto p = kv_map_.emplace(key, tInternalItem {key, item});
    if (!p.second) {
      LOG(ERROR) << "Could not add New item for key: " << key;
      return kBestNInsertStatusDidNotAddFailure;
    }

    // Insert the item to the heap.
    ra_heap_.insert(&(p.first->second));
    return kBestNInsertStatusAddedNew;
  }

  void RemoveTopItem() {
    tInternalItem* item = ra_heap_.pop();
    kv_map_.erase(item->key);
  }

  BestNInsertStatus CheckAndUpdateExistingItem(const string& key, const T& item) {
    auto iter = kv_map_.find(key);
    if (iter == kv_map_.end()) return kBestNInsertStatusInvalid;

    // Compare this item with the existing item, merely being equal is not enough.
    if (!comp_(item, iter->second.data)) return kBestNInsertStatusDidNotAddExistingBetter;

    // The new item is better, lets update it.
    iter->second.data = item;
    ra_heap_.update(&(iter->second));
    return kBestNInsertStatusAddedExistingWorse;
  }

  // Rebuilds the heap.
  void RebuildHeap() {
    for (auto& p : kv_map_) ra_heap_.insert(&(p.second));
  }
  // The capacity of the the container.
  const int capacity_;

  // Map for associating key with the corresponding value.
  unordered_map<string, tInternalItem> kv_map_;

  // The random access heap associated with the data.
  RAHeap<tInternalItem, compare_member<tInternalItem, T, &tInternalItem::data,
      Comparator>> ra_heap_;

  // The cocomp_ator used to evaluate values.
  Comparator comp_;
};

}  // namespace tl


#endif  // _PUBLIC_UTIL_TEMPLATES_CONTAINER_BEST_N_H_
