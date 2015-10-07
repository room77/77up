#ifndef _PUBLIC_UTIL_MEMORY_STRINGSTORAGE_H_
#define _PUBLIC_UTIL_MEMORY_STRINGSTORAGE_H_

// Big warehouse to provide efficient string storage in memory
//
// It stores all null-terminated strings in a big block of memory, and
// returns char pointers that will remain valid as long as the StringStorage
// object is valid.
//
// Duplicate strings are stored only once.

// Usage:
//
//   char *s = gStringStorage.Store("foobar");
//
// The method returns a valid char pointer to a copy of "foobar" string.
// There is no need to allocate/free space.

#include <atomic>
#include <mutex>
#include <unordered_set>

#include "base/common.h"
#include "util/hash/hash_util.h"
#include "util/string/strutil.h"

template<class Hash = ::hash::str_hash, class Equal = ::hash::str_eq>
class StringStorage_Base {
 public:
  StringStorage_Base() : locked_(false) {};
  ~StringStorage_Base() {
    // clear the index
    index_string_.clear();
    // free all space allocated
    for (int i = 0; i < storage_.size(); i++)
      delete storage_[i];
    storage_.clear();
  }

  // Store() method takes three types of arguments:
  //   char pointer, string, or char pointer + length
  const char *Store(const char *str);
  inline const char *Store(const string& str) { return Store(str.c_str()); }
  inline const char *Store(const char *str, int len) {
    string s(str, len);
    return Store(s.c_str());
  }

  // Lookup() performs lookup only, and returns NULL if the string is not found
  inline const char *Lookup(const char *str) {
    bool locked_mutex = false;
    if (!locked_) {
      mutex_.lock();
      locked_mutex = true;
    }
    typename tStringIndex::const_iterator i = index_string_.find(str);
    const char* ret = nullptr;
    if (i != index_string_.end())
      ret = *i;  // string found -- return the existing pointer

    if (locked_mutex) mutex_.unlock();

    return ret;
  }
  inline const char *Lookup(const string& str) { return Lookup(str.c_str()); }
  inline const char *Lookup(const char *str, int len) {
    string s(str, len);
    return Lookup(s.c_str());
  }

  // Lock the stringstorage class.
  void Lock() { locked_ = true; }

 private:
  // memory blocks to store actual strings
  static const int gMemoryBlockSize = 1048576;
  typedef struct {
    char mem[gMemoryBlockSize];
    int num_used;
  } tMemoryBlock;
  vector<tMemoryBlock *> storage_;

  // hash index into the string storage structure
  typedef unordered_set<const char *, Hash, Equal> tStringIndex;
  tStringIndex index_string_;
  bool locked_;
  mutex mutex_;
};

template<class Hash, class Equal>
const char *StringStorage_Base<Hash, Equal>::Store(const char *str) {

  // Do not allow a store after the storage is locked.
  ASSERT(locked_ == false);

  // check if the string exists
  const char *existing_string = Lookup(str);
  if (existing_string != NULL)
    return existing_string;  // found the string

  // string not found

  // find space for the new string
  int space_needed = strlen(str) + 1;
  ASSERT(space_needed < gMemoryBlockSize);

  lock_guard<mutex> l(mutex_);
  // first, search in existing blocks, starting from the most recent one
  for (int i = storage_.size() - 1; i >= 0; i--) {
    int num_used = storage_[i]->num_used;
    if (num_used + space_needed <= gMemoryBlockSize) {
      char *new_str = &(storage_[i]->mem[num_used]);
      strcpy(new_str, str);

      storage_[i]->num_used += space_needed;
      ASSERT(storage_[i]->num_used <= gMemoryBlockSize);

      // index the new string
      index_string_.insert(new_str);

      return new_str;
    }
  }

  // All blocks are full.  Need to allocate a new block

  VLOG(4) << "StringStorage is allocating block " << storage_.size();

  tMemoryBlock *blk = new tMemoryBlock;
  char *new_str = blk->mem;
  strcpy(new_str, str);
  blk->num_used = space_needed;

  // index the new string
  index_string_.insert(new_str);

  storage_.push_back(blk);
  return new_str;
}


typedef StringStorage_Base<> StringStorage;
typedef StringStorage_Base<::hash::str_casefold_hash, ::hash::str_casefold_eq>
          StringStorage_CaseInsensitive;

// global variable to store strings efficiently
extern StringStorage_CaseInsensitive gStringStorage;

#endif  // _PUBLIC_UTIL_MEMORY_STRINGSTORAGE_H_
