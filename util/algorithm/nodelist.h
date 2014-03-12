#ifndef _UTIL_NODELIST_H_
#define _UTIL_NODELIST_H_

#include "base/common.h"

// Heap implementation of a priority queue to handle a list of nodes.
//

// This class is designed to keep track of a list of "nodes" as part of A*
// algorithm implementation.
//
// The "node" struct must contain the following two fields:
//
//   float f_value;       // the node's f-value (cumulative cost plus
//                        // estimated remaining cost), as computed with
//                        // its A* heuristics function.
//
//   int nodelist_index;  // used by NodeList for internal bookkeeping
//

template <class T>
class NodeList {
 public:
  NodeList() {}
  ~NodeList() { Clear(); }

  // clear the internal heap
  inline void Clear() { heap_.clear(); }

  // reserve space for vector heap_
  inline void Reserve(int n) { heap_.reserve(n); }

  // return the total number of nodes
  inline int size() const { return heap_.size(); };

  // return the Nth node
  inline T *NthNode(int n) const {
    return ((n >= 0 && n < heap_.size()) ? heap_[n] : nullptr);
  };

  // current, parent, left-child and right-child nodes
  inline T *Current(int n) const { return NthNode(n); }
  inline T *Parent(int n) const { return NthNode((n - 1) / 2); }
  inline T *LeftChild(int n) const { return NthNode(n * 2 + 1); }
  inline T *RightChild(int n) const { return NthNode(n * 2 + 2); }

  // Add a new node to the node list (heap), while maintaining the heap
  // property that a parent's f-value must be <= its children's
  inline void AddNode(T *newnode) {
    ASSERT(newnode != nullptr);

    heap_.push_back(newnode);
    int current_idx = heap_.size() - 1;

    // move the new node up to an appropriate position in the heap
    newnode->nodelist_index = current_idx;
    MoveUpItem(current_idx);
  }

  // swap a node with its parent node
  void SwapWithParent(int idx) {
    ASSERT(idx > 0);
    int parent_idx = (idx - 1) / 2;

    T *tmp = heap_[parent_idx];
    heap_[parent_idx] = heap_[idx];
    heap_[idx] = tmp;

    // reset nodelist_index values
    heap_[parent_idx]->nodelist_index = parent_idx;
    heap_[idx]->nodelist_index = idx;
  }

  // Move a node up to its appropriate position in the heap, maintaining
  // the heap property that a parent's f-value is always <= its children's
  void MoveUpItem(int idx) {
    int current_idx = idx;
    T *current = Current(idx);

    // sanity check: child nodes' f_values must be equal or greater
    T *left_child = LeftChild(idx);
    ASSERT(left_child == nullptr || left_child->f_value >= current->f_value);
    T *right_child = RightChild(idx);
    ASSERT(right_child == nullptr || right_child->f_value >= current->f_value);

    while (current_idx > 0) {
      int parent_idx = (current_idx - 1) / 2;
      if (heap_[parent_idx]->f_value <= heap_[current_idx]->f_value)
        break;
      else {
        SwapWithParent(current_idx);  // swap current node with its parent node
        current_idx = parent_idx;  // go to the parent node
      }
    }
  }

  // Move a node down to its appropriate position in the heap, maintaining
  // the heap property that a parent's f_value is always smaller than its
  // children's
  void MoveDownItem(int idx) {
    int current_idx = idx;
    int num_nodes = heap_.size();

    // sanity check: parent node's f_value must be <= current node's f_value
    ASSERT(idx == 0 || Parent(idx)->f_value <= Current(idx)->f_value);

    while (current_idx < num_nodes) {
      int left_child_idx = current_idx * 2 + 1;
      int right_child_idx = current_idx * 2 + 2;

      if (right_child_idx < num_nodes) {
        // right child exists in the tree (so both children exist)
        if (heap_[left_child_idx]->f_value <=
            heap_[right_child_idx]->f_value) {
          // both children exists; left child has lower f_value
          if (heap_[current_idx]->f_value <= heap_[left_child_idx]->f_value)
            break;  // done
          else {
            // swap current node with its left child node
            SwapWithParent(left_child_idx);
            // go to the left child node
            current_idx = left_child_idx;
          }
        }
        else {
          // both children exists; right child has lower f_value
          if (heap_[current_idx]->f_value <= heap_[right_child_idx]->f_value)
            break;  // done
          else {
            // swap current node with its right child node
            SwapWithParent(right_child_idx);
            // go to the right child node
            current_idx = right_child_idx;
          }
        }
      }
      else {
        // right child does not exist in the tree
        if (left_child_idx < num_nodes) {
          // right child does not exist in the tree but left child exists
          if (heap_[current_idx]->f_value <= heap_[left_child_idx]->f_value)
            break;  // done
          else {
            // swap current node with its left child node
            SwapWithParent(left_child_idx);
            break;  // done
          }
        }
        else  // neither child exists in the tree
          break;  // done
      }
    }
  }

  // remove an item from the node list
  inline void RemoveItem(int idx) {
    ASSERT(idx >= 0 && idx < heap_.size());
    // replace this node with the last node in the heap
    heap_[idx] = heap_.back();
    heap_[idx]->nodelist_index = idx;
    heap_.pop_back();
    if (idx < heap_.size())
      AdjustItemLocation(idx);
  }

  // move a node up or down to its appropriate position
  inline void AdjustItemLocation(int idx) {
    ASSERT(idx >= 0 && idx < heap_.size());
    if (idx == 0 || Parent(idx)->f_value <= Current(idx)->f_value)
      MoveDownItem(idx);
    else
      MoveUpItem(idx);
  }

  // move a node up or down to its appropriate position
  inline bool AdjustPosition(T *item) {
    int idx = item->nodelist_index;
    if (idx >= 0 && idx < heap_.size() && heap_[idx] == item) {
      AdjustItemLocation(idx);
      return true;
    }
    else
      return false;  // item not in nodelist
  }

  // check if the item is in this open list
  inline bool InList(const T *item) const {
    int idx = item->nodelist_index;
    return (idx >= 0 && idx < heap_.size() && heap_[idx] == item);
  }

  // replace an old item with a new one
  inline void ReplaceItem(T *old_item, T *new_item) {
    int old_index = old_item->nodelist_index;
    ASSERT(old_index >= 0 && old_index < heap_.size());
    ASSERT(heap_[old_index] == old_item);

    *old_item = *new_item;  // replace item content
    old_item->nodelist_index = old_index;
    AdjustItemLocation(old_index);  // adjust its position in the open list
  }

  // get the node with the lowest f_value, and remove it from the node list
  // (return nullptr if the list is empty; otherwise, return the node)
  inline T *GetAndRemoveFirstNode() {
    if (heap_.empty()) return nullptr;  // list is already empty

    T *ret_node = heap_[0];

    // clear the index field since we are removing the item
    ret_node->nodelist_index = -1;

    // remove the first node from the node list
    RemoveItem(0);

    return ret_node;
  }

  // get the first node, but do not remove it
  inline const T *FirstNode() const {
    if (heap_.empty())
      return nullptr;  // list is already empty
    else
      return heap_[0];  // return the first node from node list
  }

  // for debugging only - check heap consistency
  inline void CheckHeapConsistency() const {
    LOG(INFO) << "Checking heap consistency...";
    for (int i = heap_.size() - 1; i > 0; i--)
      ASSERT(Current(i)->f_value >= Parent(i)->f_value);
  }

 private:
  std::vector<T *> heap_;
};

#endif
