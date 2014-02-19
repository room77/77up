#ifndef _PUBLIC_UTIL_INDEX_INDEX_H_
#define _PUBLIC_UTIL_INDEX_INDEX_H_

#include "base/common.h"
#include "util/hash/hash_util.h"
#include "util/memory/memory_block.h"
#include "util/serial/serializer.h"
#include "util/memory/stringstorage.h"

//
// Index class supports index creation for any key-value pair.
//
// For efficiency reasons, keys and values are often pointers to some
// persistent storage, such as UniqueCollection or gStringStorage.
// (See examples below.)  Pointers are stored as-is, and it is the caller's
// responsibility to make sure that all pointer point to valid locations
// throughout the lifetime of the index.
//
// HeavyIndex is similar to Index in terms of functionality, but it is
// optimized for the case where the same key corresponds to a large number of
// value items.
//

//
// IMPORTANT: The index's key type can be a number or a pointer.  It can also
//            be a "pair" or struct of those basic types.  In case of
//            a pointer (such as char * or Point *), equality test is
//            performed on the pointer directly.  Two pointers are equal
//            if and only if they point to the same location.
//
//            If "const char *" is used as key type, the index automatically
//            uses gStringStorage (case-insensitive global storage) to
//            "normalize" the pointer, so that internal pointer comparisons
//            would make sense.  If any other pointer is used as key type,
//            it is the caller's responsibility to normalize them (possibly
//            with the help of UniqueCollection).  For example, to index
//            on Point type, we can use pointers returned by gPointCollection.
//            This can also help ensure that these key pointers point to valid
//            locations throughout the lifetime of the Index object, which is
//            required.
//
//            Do not use "string" or "char *" as index key type.  Use
//            "const char *" instead.
//
//            Do not use any non-flat data type as the index key type.
//

// Usage example:
//
// Suppose we already have a vector of roads:
//   vector<const Road *> road_list;  // all roads
//
// To create a secondary index on end-points, here is what we can do:
//
// Step 1: define the index function (possibly in .h file)
//
//   struct index_by_end_points {
//     void operator()(const Road *r,
//                     vector<const Point *> *result) const {
//       result->push_back(gPointCollection.Store(*(r->get_start())));
//       result->push_back(gPointCollection.Store(*(r->get_end())));
//     }
//   }
//
//   Note: The type of the first argument of operator() must be
//         the Container's object data type.
//           (In this example, vector<Road *> 's data type is Road *.)
//         The type of the second argument must be a vector of pointers
//         to the key type.  (Key-type equality is defined by pointer
//         equality; therefore, it is recommended to have these pointers
//         point to UniqueCollection members.)
//         "Flat" structures like pair<> or struct may be used, but
//         "deep" structures like string or vector may not be used.
//
// Step 2: Declare the index (possibly in .h file)
//
//   Index<const Point *, const Road *> end_point_index;
//
// Step 3: Initialize the index (possibly in .cc file)
//
//   end_point_index.Init<index_by_end_points>(road_list);
//                                     // road_list is of vector<Road *> type
//
// -- Note: road_list must support const_iterator.
//          Another way to initialize is to replace Steps 1 and 3 by this loop:
//             for (vector<const Road *>::const_iterator i = road_list.begin();
//                  i != road_list.end();
//                  ++i) {
//               end_point_index.AddToIndex
//                 (gPointCollection.Store(*(r->get_start())), *i);
//               end_point_index.AddToIndex
//                 (gPointCollection.Store(*(r->get_end())), *i);
//             }
//
//
// Finally, here is how to retrieve from the index:
//
//   const Point *point1 = ...;
//   vector<const Road *> result;
//   int num_items = index.Retrieve(point1, &result);
//
// If we know there is at most one match, we can also do this:
//
//   const Road *result = index.RetrieveUnique(point1);
//
//
// For HeavyIndex, we can also retreive the unordered_set directly and use
// const_iterator to manipulate the result:
//
//     const IndexType::tValueSet *result = index2.RetrieveSet(point1);
//
// (where IndexType is the type of the index defined in Step 2 above)
//
//
// -----------------------------------------------------------------------
//
// For simple indexes (specifically, indexes on one field of a struct
// inside a UniqueCollection or Collection), we have a macro
// BUILD_INDEX_ON_FIELD that can help.  This macro is defined later in this
// file, and has detailed comments about its usage.
//
// -----------------------------------------------------------------------
//


// global storage normalization:
//   convert a standard string (const char *) to pointer to global storage;
//   keep all other data types as-is
namespace GlobalStorage {

  //
  // Normalize_RW may change gStringStorage, and is intended for AddToIndex
  // (if input is const char *, return value is guaranteed to point to an
  //  idential string, though not necessarily at the same memory location)
  //

  // for const char * or char * or string, normalize using gStringStorage
  inline const char *Normalize_RW(const char *input) {
    return gStringStorage.Store(input);
  }
  inline const char *Normalize_RW(char *input) {
    return gStringStorage.Store(input);
  }
  inline const char *Normalize_RW(const string& input) {
    return gStringStorage.Store(input);
  }

  // for most types, simply return the input unchanged
  template<class T>
  inline T Normalize_RW(const T& input) { return input; }

  // normalize pair<string, string> to pair<const char *, const char *>
  inline pair<const char *, const char *>
  Normalize_RW(const pair<string, string>& input) {
    return
      pair<const char *, const char *>(Normalize_RW(input.first.c_str()),
                                       Normalize_RW(input.second.c_str()));
  }

  // normalize pair<string, T> to pair<const char *, T>
  template<class T>
  inline pair<const char *, T> Normalize_RW(const pair<string, T>& input) {
    return pair<const char *, T>(Normalize_RW(input.first.c_str()),
                                 Normalize_RW(input.second));
  }

  // normalize pair<T, string> to pair<T, const char *>
  template<class T>
  inline pair<T, const char *>
    Normalize_RW(const pair<T, string>& input) {
    return pair<T, const char *>(Normalize_RW(input.first),
                                 Normalize_RW(input.second.c_str()));
  }

  // for pairs of two fields, normalize each one individually
  template<class T1, class T2>
  inline pair<T1, T2> Normalize_RW(const pair<T1, T2>& input) {
    return pair<T1, T2>(Normalize_RW(input.first), Normalize_RW(input.second));
  }


  // normalize pair<string, pair<string, string> to
  // pair<const char *, pair<const char *, const char *> >
  inline pair<const char *, pair<const char *, const char *> >
  Normalize_RW(const pair<string, pair<string, string> >& input) {
    return make_pair(Normalize_RW(input.first.c_str()),
                     Normalize_RW(input.second));
  }

  // normalize pair<T, pair<string, string> to
  // pair<T, pair<const char *, const char *> >
  template<class T>
  inline pair<T, pair<const char *, const char *> >
  Normalize_RW(const pair<T, pair<string, string> >& input) {
    return make_pair(Normalize_RW(input.first),
                     Normalize_RW(input.second));
  }

  //
  // Normalize_R never changes gStringStorage, and is intended for Retrieve
  // (if input is const char *, return value may be NULL if the string doesn't
  //  exist in gStringStorage, and therefore cannot exist in the index)
  //

  // for const char * or char * or string, normalize using gStringStorage
  inline const char *Normalize_R(const char *input) {
    return gStringStorage.Lookup(input);
  }
  inline const char *Normalize_R(char *input) {
    return gStringStorage.Lookup(input);
  }
  inline const char *Normalize_R(const string& input) {
    return gStringStorage.Lookup(input);
  }

  // for most types, simply return the input unchanged
  template<class T>
  inline T Normalize_R(const T& input) { return input; }

  // normalize pair<string, string> to pair<const char *, const char *>
  inline pair<const char *, const char *>
  Normalize_R(const pair<string, string>& input) {
    return
      pair<const char *, const char *>(Normalize_R(input.first.c_str()),
                                       Normalize_R(input.second.c_str()));
  }

  // normalize pair<string, T> to pair<const char *, T>
  template<class T>
  inline pair<const char *, T> Normalize_R(const pair<string, T>& input) {
    return pair<const char *, T>(Normalize_R(input.first.c_str()),
                                 Normalize_R(input.second));
  }

  // normalize pair<T, string> to pair<T, const char *>
  template<class T>
  inline pair<T, const char *> Normalize_R(const pair<T, string>& input) {
    return pair<T, const char *>(Normalize_R(input.first),
                                 Normalize_R(input.second.c_str()));
  }

  // for pairs of two fields, normalize each one individually
  template<class T1, class T2>
  inline pair<T1, T2> Normalize_R(const pair<T1, T2>& input) {
    return pair<T1, T2>(Normalize_R(input.first), Normalize_R(input.second));
  }

  // normalize pair<string, pair<string, string> to
  // pair<const char *, pair<const char *, const char *> >
  inline pair<const char *, pair<const char *, const char *> >
  Normalize_R(const pair<string, pair<string, string> >& input) {
    return make_pair(Normalize_R(input.first.c_str()),
                     Normalize_R(input.second));
  }

  // normalize pair<T, pair<string, string> to
  // pair<T, pair<const char *, const char *> >
  template<class T>
  inline pair<T, pair<const char *, const char *> >
  Normalize_R(const pair<T, pair<string, string> >& input) {
    return make_pair(Normalize_R(input.first),
                     Normalize_R(input.second));
  }
}


template<class KeyType, class ValueType>
class IndexBase {
 public:
  IndexBase(const string& name = "")
    : max_items_per_key_(-1), name_(name) {}
  virtual ~IndexBase() {}

  // get name for debugging purposes
  inline void set_name(const string& name) { name_ = name; }
  inline string get_name() const { return name_; }

  inline void set_max_items_per_key(int max) { max_items_per_key_ = max; }
  inline int get_max_items_per_key() const   { return max_items_per_key_; }

  // initialize index for a container
  // (IndexSpec defines operator() that takes two arguments: the first one
  //  is input, of the Container's data type, and the second one is output,
  //  a pointer to a vector of KeyType elements)
  template<class IndexSpec, class Container>
  void Init(const Container& container) {
    // initialize the index
    VLOG(2) << "Building index: " << name_ << "...";
    Clear();
    int count = 0;
    for (typename Container::const_iterator i = container.begin();
         i != container.end();
         ++i) {
      // for each item in the container
      vector<KeyType> result;
      IndexSpec()(*i, &result);
      // IndexSpec fills a vector of key pointers.
      // We need to index each one of them.
      for (int j = 0; j < result.size(); ++j) {
        AddToIndex(result[j], *i);
        count++;
      }
    }
    VLOG(2) << "Done.  (container size = " << container.size()
           << "; index size = " << count << ")";
  }

  virtual void Clear() = 0;

  // Given a signature and a corresponding object, add them to the index.
  template<class RawKeyType>
  inline void AddToIndex(const RawKeyType& key,
                         const ValueType& candidate,
                         bool unique_key = false) {
    KeyType normalized_key = GlobalStorage::Normalize_RW(key);
    AddKeyValueToIndex(normalized_key, candidate, unique_key);
  }

  template<class RawKeyType>
  void RemoveFromIndex(const RawKeyType& key, const ValueType& candidate) {
    KeyType normalized_key = GlobalStorage::Normalize_RW(key);
    RemoveKeyValueFromIndex(normalized_key, candidate);
  }

 protected:
  virtual void AddKeyValueToIndex(const KeyType& normalized_key,
                                  const ValueType& candidate,
                                  bool unique_key) = 0;

  virtual void RemoveKeyValueFromIndex(const KeyType& normalized_key,
                                  const ValueType& candidate) {
    LOG(INFO) << "This function is not supported";
    ASSERT(0);
  }

  int max_items_per_key_;

 private:
  string name_;
};


// prohibit "string" type from being used as index key (individually or as
// part of a pair)
template<class V>
class IndexBase<string, V> {
 private:
  IndexBase(const string& name = "") {}  // see comments above
};
template<class V, class W>
class IndexBase<pair<string, W>, V> {
 private:
  IndexBase(const string& name = "") {}  // see comments above
};
template<class V, class W>
class IndexBase<pair<W, string>, V> {
 private:
  IndexBase(const string& name = "") {}  // see comments above
};

// prohibit "char *" type from being used as index key (individually or as
// part of a pair)   --- use const char * instead
template<class V>
class IndexBase<char *, V> {
 private:
  IndexBase(const string& name = "") {}  // see comments above
};
template<class V, class W>
class IndexBase<pair<char *, W>, V> {
 private:
  IndexBase(const string& name = "") {}  // see comments above
};
template<class V, class W>
class IndexBase<pair<W, char *>, V> {
 private:
  IndexBase(const string& name = "") {}  // see comments above
};


template<class KeyType, class ValueType>
class Index : public IndexBase<KeyType, ValueType> {
 private:
  // index storage structure
  typedef unordered_multimap<KeyType, ValueType, ::hash::flat_hash<KeyType>,
      ::hash::flat_eq<KeyType> > tIndexType;
  tIndexType index_;

  //
  // public methods
  //

 public:
  Index(const string& name = "") : IndexBase<KeyType, ValueType>(name) {};
  virtual ~Index() {};

  // returns the total number of items indexed
  inline int size() const { return index_.size(); }

  virtual void Clear() {
    index_.clear();
  }

  // Given a signature key, retrieve all items indexed by the signature
  // (the second parameter is a pointer to the result container, which can
  //  be of any type that supports clear(), insert() and end().  A common
  //  example would be vector<>.)
  template<class RawKeyType, class ResultContainer>
  int Retrieve(const RawKeyType& key, ResultContainer *result) const {
    KeyType normalized_key = GlobalStorage::Normalize_R(key);
    pair<typename tIndexType::const_iterator,
         typename tIndexType::const_iterator>
      p = index_.equal_range(normalized_key);
    result->clear();
    for (typename tIndexType::const_iterator i = p.first;
         i != p.second;
         ++i)
      result->insert(result->end(), i->second);
    return result->size();
  }

  // Given a key, retrieve the one single item indexed by this key
  // (if there is more than one match, report an assertion error;
  // if there is no match, return NULL)
  template<class RawKeyType>
  ValueType RetrieveUnique(const RawKeyType& key) const {
    vector<ValueType> result;
    ASSERT(Retrieve(key, &result) <= 1)
      << "Multiple matches found when calling RetrieveUnique";
    if (result.size() == 0)  // no match found
      return nullptr;
    else
      return result[0];
  }

 protected:
  // Given a signature and a corresponding object, add them to the index.
  virtual void AddKeyValueToIndex(const KeyType& normalized_key,
                                  const ValueType& candidate,
                                  bool unique_key) {
    // check if we already have the mapping in our index
    pair<typename tIndexType::const_iterator,
         typename tIndexType::const_iterator>
      p = index_.equal_range(normalized_key);

    int count = 0;
    for (typename tIndexType::const_iterator i = p.first;
         i != p.second;
         ++i) {
      if (i->second == candidate)
        return;  // do nothing if we already have this mapping
      ++count;
      if (IndexBase<KeyType, ValueType>::max_items_per_key_ > 0 &&
          count >= IndexBase<KeyType, ValueType>::max_items_per_key_)
        return;  // do nothing if we already have enough items for this key
    }

    if (unique_key)
      ASSERT(p.first == p.second)
        << "index " << IndexBase<KeyType, ValueType>::get_name()
        << ": key is not unique";  // << serial::Serializer::ToJSON(normalized_key);

    // insert the new item to the index
    index_.insert(typename tIndexType::value_type(normalized_key, candidate));

    if (size() % 1000000 == 0)
      VLOG(2) << "... built index for " << size() << " items";
  }

  virtual void RemoveKeyValueFromIndex(const KeyType& normalized_key,
                                       const ValueType& candidate) {
    // check if the key is in our index.
    pair<typename tIndexType::const_iterator,
        typename tIndexType::const_iterator> p =
             index_.equal_range(normalized_key);

    for (typename tIndexType::const_iterator i = p.first; i != p.second; ++i) {
      if (i->second == candidate) {
        index_.erase(i);
        break;
      }
    }
  }
};


//
// HeavyIndex: same as index, but use unordered_map of unordered_set to handle
//   duplicate keys.  (Optimized for the case where there are lots of
//   duplicate keys.)
//
template<class KeyType, class ValueType>
class HeavyIndex : public IndexBase<KeyType, ValueType> {
 public:
  // type of each index entry is a unordered_set
  typedef unordered_set<ValueType, ::hash::flat_hash<ValueType>,
      ::hash::flat_eq<ValueType> > tValueSet;

 private:
  // index storage structure
  typedef unordered_map<KeyType, tValueSet *, ::hash::flat_hash<KeyType>,
      ::hash::flat_eq<KeyType> > tIndexType;
  tIndexType index_;

  MemoryBlock<tValueSet> memory_;

  int size_;

  //
  // public methods
  //

 public:
  HeavyIndex(const string& name = "") : IndexBase<KeyType, ValueType>(name) {
    memory_.set_name(name);
    size_ = 0;
  };
  virtual ~HeavyIndex() {};

  // returns the total number of keys indexed
  inline int NumKeys() const { return index_.size(); }
  inline int size() const { return size_; }

  virtual void Clear() {
    index_.clear();
    memory_.Clear();
  }

  // Given a signature key, retrieve all items indexed by the signature
  // (return value is a pointer to a unordered_set of values)
  template<class RawKeyType>
  const tValueSet *RetrieveSet(const RawKeyType& key) const {
    KeyType normalized_key = GlobalStorage::Normalize_R(key);
    static const tValueSet empty_collection;
    typename tIndexType::const_iterator i = index_.find(normalized_key);
    if (i == index_.end()) {
      return &empty_collection;
    }
    else {
      return i->second;
    }
  }

  // Given a signature key, retrieve all items indexed by the signature
  // (the second parameter is a pointer to the result container, which can
  //  be of any type that supports clear(), insert() and end().  A common
  //  example would be vector<>.)
  template<class RawKeyType, class ResultContainer>
  int Retrieve(const RawKeyType& key, ResultContainer *result) const {
    const tValueSet *s = RetrieveSet(key);
    result->clear();
    result->reserve(s->size());
    for (typename tValueSet::const_iterator i = s->begin();
         i != s->end();
         ++i) {
      result->insert(result->end(), *i);
    }
    return result->size();
  }

  // Given a key, retrieve the one single item indexed by this key
  // (if there is more than one match, report an assertion error;
  // if there is no match, return NULL)
  template<class RawKeyType>
  ValueType RetrieveUnique(const RawKeyType& key) const {
    const tValueSet *result = RetrieveSet(key);
    ASSERT(result->size() <= 1)
      << "Multiple matches found when calling RetrieveUnique";
    if (result->size() == 0)  // no match found
      return NULL;
    else
      return *(result->begin());
  }

 protected:
  // Given a signature and a corresponding object, add them to the index.
  virtual void AddKeyValueToIndex(const KeyType& normalized_key,
                                  const ValueType& candidate,
                                  bool unique_key) {
    ASSERT(!unique_key)
      << "Please use Index instead of HeavyIndex "
      << "if you require the key to be unique";
    typename tIndexType::iterator i = index_.find(normalized_key);
    tValueSet *vs;
    if (i == index_.end()) {
      // create a new value set for this signature key
      // start with as few buckets as possible (even though we specify
      // 1 as number of buckets, unordered_set will choose a larger number of
      // buckets)
      vs = new (memory_.Alloc()) tValueSet(1);
      index_[normalized_key] = vs;
    }
    else {
      // find the existing value set for this signature key
      vs = i->second;
      // check if we already have enough items for this key
      if (IndexBase<KeyType, ValueType>::max_items_per_key_ > 0 &&
          vs->size() >= IndexBase<KeyType, ValueType>::max_items_per_key_)
        return;
    }
    vs->insert(candidate);
    ++size_;
    if (size_ % 1000000 == 0)
      VLOG(2) << "... built index for " << size_ << " items";
  }
};




//
// macros to index a Collection (or UniqueCollection) based on one field
//
// Note: an index must be declared with its value type matching the
//       collection's value type, and its key type matching the field's type
//       (key type: we support numbers, strings or pointers;  string fields
//        must correspond to "const char *" as index key type.  See example
//        below.)
//
// for example:
//   typedef struct {
//     int a, b;
//     string c;
//     SIGNATURE(a*1 / b*2);
//   } tFoo;
//   UniqueCollection<tFoo> coll;
//   Index<int, const tFoo *> index1_on_coll;
//   Index<const char *, const tFoo *> index2_on_coll;
//
// int init_main() {
//   ...
//   ...
//   BUILD_INDEX_ON_FIELD(coll, a, index1_on_coll);
//   BUILD_INDEX_ON_FIELD(coll, c, index2_on_coll);
//   ...
//   ...
// }
//
// BUILD_UNIQUE_INDEX_ON_FIELD is similar to BUILD_INDEX_ON_FIELD, except
// that every field must contain a unique value (assertion failure if duplicate
// field values are encountered)
//
// We also provide macros to build index on a pair of two fields
//
// BUILD_PREFIX_INDEX_ON_FIELD builds indices on string prefixes (field type
// must be string or char *; only non-space characters are indexed)
//

#define BUILD_INDEX_ON_FIELD(collection, field, index) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    (index).AddToIndex((*__i)->field, *__i, false); \
  }

#define BUILD_UNIQUE_INDEX_ON_FIELD(collection, field, index) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    (index).AddToIndex((*__i)->field, *__i, true); \
  }

#define BUILD_INDEX_ON_TWO_FIELDS(collection, field1, field2, index) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    (index).AddToIndex(make_pair((*__i)->field1, (*__i)->field2), \
                       *__i, false); \
  }

#define BUILD_UNIQUE_INDEX_ON_TWO_FIELDS(collection, field1, field2, index) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    (index).AddToIndex(make_pair((*__i)->field1, (*__i)->field2), \
                       *__i, true); \
  }

#define BUILD_PREFIX_INDEX_ON_FIELD(collection, field, index) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    const string& __s = (*__i)->field; \
    string __p; \
    (index).AddToIndex(__p, *__i, false); \
    for (int __j = 0; __j < __s.size(); __j++) { \
      char __c = __s[__j]; \
      if (IS_ALPHANUMERIC_INTL(__c)) { \
        __p += __c; \
        (index).AddToIndex(__p, *__i, false); \
      } \
    } \
  }

// The macros below allow a specific value to be ignored while building index.
// This is usually the invalid value for the field. e.g. "" (empty string) for
// a string field.
#define BUILD_INDEX_ON_FIELD_IGNORE_VALUE(collection, field, index, ignore) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    if ((*__i)->field == ignore) continue; \
    (index).AddToIndex((*__i)->field, *__i, false); \
  }

#define BUILD_UNIQUE_INDEX_ON_FIELD_IGNORE_VALUE(collection, field, index, ignore) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    if ((*__i)->field == ignore) continue; \
    (index).AddToIndex((*__i)->field, *__i, true); \
  }

#define BUILD_INDEX_ON_TWO_FIELDS_IGNORE_VALUES(collection, field1, field2, index, ignore1, ignore2) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    if ((*__i)->field1 == ignore1 || (*__i)->field2 == ignore2) continue; \
    (index).AddToIndex(make_pair((*__i)->field1, (*__i)->field2), \
                       *__i, false); \
  }

#define BUILD_UNIQUE_INDEX_ON_TWO_FIELDS_IGNORE_VALUES(collection, field1, field2, index, ignore1, ignore2) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    if ((*__i)->field1 == ignore1 || (*__i)->field2 == ignore2) continue; \
    (index).AddToIndex(make_pair((*__i)->field1, (*__i)->field2), \
                       *__i, true); \
  }

// The macros below are specifically when both the fields are known to be
// of type string. We want to normalize the string using 'norm' before we use
// it as a key for indexing. In addition we also ignore empty strings.
#define BUILD_INDEX_ON_STR_FIELD_INTERNAL(collection, field, index, norm, uniq) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    if ((*__i)->field == "") continue; \
    (index).AddToIndex(norm((*__i)->field), *__i, uniq); \
  }

#define BUILD_INDEX_ON_STR_FIELD(collection, field, index, norm) \
    BUILD_INDEX_ON_STR_FIELD_INTERNAL(collection, field, index, norm, false)

#define BUILD_UNIQUE_INDEX_ON_STR_FIELD(collection, field, index, norm) \
    BUILD_INDEX_ON_STR_FIELD_INTERNAL(collection, field, index, norm, true)

#define BUILD_INDEX_ON_TWO_STR_FIELDS_INTERNAL(collection, field1, field2, index, norm, uniq) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    if ((*__i)->field1 == "" || (*__i)->field2 == "") continue; \
    (index).AddToIndex(make_pair(norm((*__i)->field1), norm((*__i)->field2)), \
                       *__i, uniq); \
  }

#define BUILD_INDEX_ON_TWO_STR_FIELDS(collection, field1, field2, index, norm) \
    BUILD_INDEX_ON_TWO_STR_FIELDS_INTERNAL(collection, field1, field2, index, norm, false)

#define BUILD_UNIQUE_INDEX_ON_TWO_STR_FIELDS(collection, field1, field2, index, norm) \
    BUILD_INDEX_ON_TWO_STR_FIELDS_INTERNAL(collection, field1, field2, index, norm, true)

#define BUILD_PREFIX_INDEX_ON_STR_FIELD(collection, field, index, norm) \
  for (auto __i = (collection).begin(); __i != (collection).end(); ++__i) { \
    const string __s = norm((*__i)->field); \
    string __p; \
    (index).AddToIndex(__p, *__i, false); \
    for (int __j = 0; __j < __s.size(); __j++) { \
      __p += __s[__j]; \
      (index).AddToIndex(__p, *__i, false); \
    } \
  }

#endif
