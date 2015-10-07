#ifndef _PUBLIC_UTIL_MEMORY_MEMORY_BLOCK_H_
#define _PUBLIC_UTIL_MEMORY_MEMORY_BLOCK_H_

#include "base/common.h"

//
// MemoryBlock class provides memory management for large collections of
// data.  It allows users to allocate memory incrementally or to delete
// everything at once, but it does not support selective deletion.
//
// Instead of calling "new" and "delete" for each object, MemoryBlock
// does "new" and "delete" on a large number of objects at once, thus
// saving time and space.
//
// Usage:
//
//   Declare MemoryBlock and specify its object type Foo:
//
//     MemoryBlock<Foo> mem;
//
//   Use Alloc to allocate space for an object, via placement new operator
//   syntax so that the object can be constructed properly
//
//     Foo *obj = new (mem.Alloc()) T(foobar);
//
//   To delete everything (and destruct all objects):
//
//     mem.Clear();
//
//   Caller does not need to worry about memory management details.
//

template<class T, int initial_size = 1>
class MemoryBlock {
 public:
  MemoryBlock() {
    initial_size_ = initial_size;
    partial_usage_ = 0;
    block_size_.clear();
    mem_.clear();
  };
  ~MemoryBlock() { Clear(); };

  // reset initial size
  void set_initial_size(int size) {
    ASSERT(block_size_.empty())
      << "initial_size must be set before MemoryBlock is used";
    initial_size_ = size;
  }

  // allocate space for one object
  T *Alloc() {
    if (mem_.empty() || partial_usage_ >= block_size_.back())
      Grow();  // allocate additional space

    ASSERT(partial_usage_ >= 0 && partial_usage_ < block_size_.back());

    partial_usage_++;
    char *block_start = mem_.back();
    return reinterpret_cast<T *>(block_start +
                                 sizeof(T) * (partial_usage_ - 1));
  }

  // delete all memory that has been allocated
  inline void Clear() {
    // first, call destructors explicitly on every item allocated
    for (int i = mem_.size() - 1; i >= 0; i--) {
      // find out how many items have been allocated for this block
      int block_usage = ((i == mem_.size() - 1) ?
                         partial_usage_ : block_size_[i]);

      char *block_start = mem_[i];
      for (int j = block_usage - 1; j >= 0; j--) {
        T *obj = reinterpret_cast<T *>(block_start + sizeof(T) * j);
        obj->~T();  // destruct this object
      }
    }

    // next, free all memory allocated
    for (int i = 0; i < mem_.size(); i++)
      delete [] (mem_[i]);
    mem_.clear();
    block_size_.clear();
    partial_usage_ = 0;
  }

  // returns the total number of blocks allocated
  inline int NumBlocks() const { return mem_.size(); }

  // set/get name of this memory block (for debugging)
  inline void set_name(const string& name) { name_ = name; }
  inline string name() const { return name_; }

 private:
  // allocate additional space
  void Grow() {
    // decide on the next block size
    // (start from initial_size_, double each time until we reach 65536,
    //  then keep constant)
    int next_block_size;
    if (block_size_.empty())
      next_block_size = initial_size_;
    else if (block_size_.back() < 65536)
      next_block_size = block_size_.back() * 2;
    else
      next_block_size = block_size_.back();

    char *new_block = new char[sizeof(T) * next_block_size];
    block_size_.push_back(next_block_size);
    mem_.push_back(new_block);
    partial_usage_ = 0;

    /*
    // for debugging: print out memory usage
    int total = 0;
    for (int i = 0; i < block_size_.size(); i++)
      total += block_size_[i];
    LOG(INFO) << "Growing " << name_ << " (sizeof " << sizeof(T) << ") * "
           << total;
    */
  }

  int initial_size_;
  int partial_usage_;
  vector<int> block_size_;
  vector<char *> mem_;
  string name_;
};


#endif  // _PUBLIC_UTIL_MEMORY_MEMORY_BLOCK_H_
