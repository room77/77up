// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// A basic random access heap.
// Note: This class is not thread safe.

#ifndef _PUBLIC_UTIL_TEMPLATES_CONTAINER_RA_HEAP_H_
#define _PUBLIC_UTIL_TEMPLATES_CONTAINER_RA_HEAP_H_

#include <algorithm>
#include <functional>

#include "base/defs.h"
#include "base/logging.h"

namespace tl {

template<typename T, typename Comparator = std::less<T>, typename Base = T,
    size_t Base::*member = &T::index>
class RAHeapBase {
 public:
  bool empty() const { return heap_.empty(); }
  int size() const { return heap_.size(); }

  // Returns the item at the given position.
  T* at(size_t pos) const { return pos < heap_.size() ? heap_[pos] : nullptr; }
  T* top() const { return at(0); }

  T* operator[](size_t pos) const { return at(pos); }

  // Push a new item to the heap.
  // Note: This does not take ownership of the pointer. The caller is responsible for ensuring
  // that the item lasts the lifetime of the heap.
  size_t push(T* item) { return insert(item); }

  // Insert a new item to the heap.
  // Note: This does not take ownership of the pointer. The caller is responsible for ensuring
  // that the item lasts the lifetime of the heap.
  size_t insert(T* item) {
    // Insert the item at the end and heapify upwards to get the right position for the item.
    heap_.push_back(item);
    set_heap_item_pos(item, size() - 1);
    return HeapifyUp(heap_.size() - 1);
  }

  T* pop() {
    T* item = heap_[0];
    erase_at(0);
    return item;
  }

  // Erases the item at the given position.
  void erase_at(size_t pos) {
    if (pos >= heap_.size()) return;
    // Move the last item to the pos and reduce the size of the heap by 1.
    heap_[pos] = heap_.back();
    heap_.pop_back();

    // Set the new index for the new item at pos.
    set_heap_item_pos(heap_[pos], pos);

    // Heapify downwords to fix the heap.
    HeapifyDown(pos);
  }

  // Erases the requested item.
  void erase(const T* item) { erase_at(heap_item_pos(item)); }

  // Updates the item at the given position.
  size_t update_at(size_t pos) {
    if (pos >= heap_.size()) return pos;
    // First heapify up.
    size_t index = HeapifyUp(pos);
    // If the item was moved up, then no need to move it down.
    if (index != pos) return index;

    // Heapify downwords to fix the heap.
    return HeapifyDown(pos);
  }

  // Updates the requested item.
  size_t update(const T* item) { return update_at(heap_item_pos(item)); }

 protected:
  size_t heap_item_pos(const T* item) const { return item->*member; }
  void set_heap_item_pos(T* item, size_t pos) const { item->*member = pos; }

  size_t get_parent_pos(size_t pos) const { return (pos - 1) / 2; }
  size_t get_left_child_pos(size_t pos) const { return 2 * pos + 1; }
  size_t get_right_child_pos(size_t pos) const { return 2 * pos + 2; }

  // Swaps two items at given indices and updates their indices.
  void swap_items(size_t pos1, size_t pos2) {
    std::swap(heap_[pos1], heap_[pos2]);
    set_heap_item_pos(heap_[pos1], pos1);
    set_heap_item_pos(heap_[pos2], pos2);
  }

  // Fixes the heap upwards from a given position.
  size_t HeapifyUp(size_t pos) {
    ASSERT(pos < heap_.size());
    // if at any point during this alg we go out of bounds, we're done.
    // Note: pos == 0 has no parent.
    while (pos > 0) {
      size_t parent_pos = get_parent_pos(pos);

      // If the comparator says current item is already correct w.r.t parent, break.
      // e.g. if comparator = std::less<T>, and item at pos is less than the parent, we are done.
      // Note: We use ! to avoid moving in case of equal values.
      if (!comp_(*heap_[parent_pos], *heap_[pos])) break;

      // Swap it up and continue with the parent's position.
      swap_items(pos, parent_pos);
      pos = parent_pos;
    }
    return pos;
  }

  size_t HeapifyDown(size_t pos) {
    // 1 to account for last left child. e.g. array [0, 1, 2, 3], we want to check parents 0 and 1.
    size_t invalid_parent_pos = get_parent_pos(heap_.size()) + 1;
    while (pos < invalid_parent_pos) {
      // Get the left child.
      size_t candidate_child = get_left_child_pos(pos);
      // Check if there is any left child.
      if (candidate_child >= heap_.size()) break;

      // Check if there is a right child and if so, if that is the one that should be compared
      // with the parent.
      size_t right_child_pos = get_right_child_pos(pos);

      if (right_child_pos < heap_.size() && comp_(*heap_[candidate_child], *heap_[right_child_pos]))
        candidate_child = right_child_pos;

      // If the comparator says current item is already correct w.r.t children, break.
      // e.g. if comparator = std::less<T>, and item at pos is greater than both the children,
      // we are done.
      // Note: We use ! to avoid moving in case of equal values.
      if (!comp_(*heap_[pos], *heap_[candidate_child])) break;

      // Swap it up and continue with the chosen childs position.
      swap_items(pos, candidate_child);
      pos = candidate_child;
    }
    return pos;
  }

  // The list that maintains the items in the heap.
  // We assume that for each a node at index i, its children are at 2i + 1 and 2i + 2. e.g.
  // node at 0 has childrent at 1 & 2, for node at 1 children are at 3 and 4 and so on.
  vector<T*> heap_;

  // The comparator for the heap. e.g. If you provide std::less<T> a max heap is created.
  Comparator comp_;
};

template<typename T, typename Comparator = std::less<T>, size_t T::*member = &T::index>
using RAHeap = RAHeapBase<T, Comparator, T, member>;

}  // namespace tl


#endif  // _PUBLIC_UTIL_TEMPLATES_CONTAINER_RA_HEAP_H_
