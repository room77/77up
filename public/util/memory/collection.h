// DEPRECATED DO NOT USE FOR NEW CODE!

// Not thread safe. Use a suitable standard stl container instead.
// Please talk to Uygar / Pramod if you have any questions.

#ifndef _PUBLIC_UTIL_MEMORY_COLLECTION_H_
#define _PUBLIC_UTIL_MEMORY_COLLECTION_H_

#include "base/common.h"
#include "util/serial/serializer.h"
#include "util/templates/functional.h"
#include "util/memory/memory_block.h"

//
// UniqueCollection class manages a collection of objects who have
// serializable signatures (as defined by SIGNATURE macro).
// It handles memory management and makes sure that no duplicate objects
// exist inside the collection
//
// (This is similar to StringStorage, except that Collection operates on
//  serializable objects, not strings.)
//
// UniqueCollection<T> 's data type is "T *".
//
//
// Collection class manages a collection of arbitrary objects, without
// uniqueness guarantee (it's implemented as a vector, so duplicates are
// allowed, and there is no way to find duplicates efficiently).  It
// handles memory management automatically.
//
// Collection<T> 's data type is "T *".
//
//
// Note: UniqueCollection's Store(), Lookup() and Insert() methods return
//       "const T *" pointers that point to the internal storage of the
//       object.  If caller needs to change the content pointed to by this
//       pointer, Lookup_Mutable() or Insert_Mutable() should be used instead
//       (they return T *); however, note that it is the caller's
//       responsibility to make sure not to touch any field that affects the
//       object's signature.  If the object's signature is changed, data
//       corruption may occur.
//


template<class T, int initial_size = 1>
class UniqueCollection {
  typedef unordered_set<T*, ::util::tl::binary_hash<T>,
      ::util::tl::binary_equal<T> > tCollection;

 public:
  UniqueCollection(const string& name = "") { set_name(name); };
  ~UniqueCollection() { Clear(); };

  // container typedefs
  typedef T object_type;
  typedef typename tCollection::value_type value_type;
  typedef typename tCollection::const_iterator const_iterator;
  // const iterators on collection_
  const_iterator begin() const { return collection_.begin(); }
  const_iterator end() const { return collection_.end(); }

  // reset initial size
  void set_initial_size(int size) { memory_.set_initial_size(size); }

  // Given a candidate object, check if it exists in our collection.
  // If so, return a pointer to the existing object
  // If not, make a copy of the object and store it in our collection, and
  //         return a pointer to the newly-created object
  inline const T *Store(const T& candidate_object) {
    return Store_Mutable(candidate_object);
  }

  // Make it pseudo compatible with other stl container types.
  const T* insert(const T& candidate_object) {
    return Store(candidate_object);
  }

  const T* insert(const_iterator iter, const T& candidate_object) {
    return Store(candidate_object);
  }


  inline T *Store_Mutable(const T& candidate_object) {
    T *item = Lookup_Mutable(candidate_object);
    if (item) {
      // This object already exists.  Return a pointer to the existing object.
      return item;
    }
    else {
      // This is a new object.  Let's make a copy and store it.
      return Insert_Mutable(candidate_object);
    }
  }

  // lookup an object.  Return NULL if not found.
  // (return value is const pointer)
  inline const T *Lookup(const T& candidate_object) const {
    return Lookup_Mutable(candidate_object);
  }

  // lookup an object.  Return NULL if not found.
  // (return pointer points to mutable value)
  //
  //    IMPORTANT: Even though caller may modify the object through this
  //               return pointer, it is the caller's responsibility to
  //               ensure that object's SIGNATURE is not changed at any
  //               time.  Data corruption may occur if caller uses
  //               Lookup_Mutable's return pointer to modify the underlying
  //               object that causes its signature to change.
  //
  inline T *Lookup_Mutable(const T& candidate_object) const {
    typename tCollection::const_iterator i =
        collection_.find(const_cast<T *>(&candidate_object));
    // note: find() doesn't really change its parameter, so the above
    //       const_cast is safe.

    if (i == collection_.end())
      return NULL;
    else
      return *i;
  }

  // Make a copy of the object and insert it into our collection
  // (return value is const pointer)
  inline const T *Insert(const T& candidate_object) {
    return Insert_Mutable(candidate_object);
  }

  // Make a copy of the object and insert it into our collection
  // (return pointer points to mutable value)
  //
  //    IMPORTANT: see warning above for Lookup_Mutable.
  //
  inline T *Insert_Mutable(const T& candidate_object) {
    // use placement syntax of operator new to call T's copy constructor
    // with new memory allocated by memory_.Alloc()
    //   (Note: destruction will be taken care of in memory_.Clear())
    T *obj = new (memory_.Alloc()) T(candidate_object);
    collection_.insert(obj);
    return obj;
  }

  // Replace an object with a new one
  // If successful, return pointer to the new object;
  // If failed (the old object cannot be found), return NULL.
  inline const T *Replace(const T& old_object, const T& new_object) {
    // look up the old object
    typename tCollection::iterator
      i = collection_.find(const_cast<T *>(&old_object));
    // note: find() doesn't really change its parameter, so the above
    //       const_cast is safe.

    if (i == collection_.end())
      return NULL;  // error!
    else {
      T *location = *i;  // save the location (for reuse later)
      collection_.erase(i);  // erase the old object from unordered_set
      *location = new_object;  // fill in the new object (reuse memory)
      collection_.insert(location);
      return location;
    }
  }

  // delete all objects from collection and free memory
  inline void Clear() {
    collection_.clear();  // remove all objects from collection_
    memory_.Clear();  // destruct all objects and free memory
  }

  // returns the total number of items stored
  inline int size() const { return collection_.size(); }

  // set/get name for debugging purposes
  inline void set_name(const string& name) {
    name_ = name;
    memory_.set_name(name);
  }
  inline string get_name() const { return name_; }

 private:

  // object collection storage structure
  tCollection collection_;
  MemoryBlock<T, initial_size> memory_;
  string name_;  // name of this collection (for debugging)
};


// --------------------------------------------------------------------


template<class T, int initial_size = 1>
class Collection {
  typedef vector<T*> tCollection;

 public:
  Collection(const string& name = "") { set_name(name); };
  ~Collection() { Clear(); }

  // container typedefs
  typedef T object_type;
  typedef typename tCollection::value_type value_type;
  typedef typename tCollection::const_iterator const_iterator;
  // const iterators on collection_
  const_iterator begin() const { return collection_.begin(); }
  const_iterator end() const { return collection_.end(); }

  // delete all objects from collection and free memory
  void Clear() {
    collection_.clear();
    memory_.Clear();
  }

  // reset initial size
  void set_initial_size(int size) { memory_.set_initial_size(size); }

  inline void reserve(int count) { collection_.reserve(count); }

  // Add a new object to the collection (make a copy of the object)
  T* Store(const T& candidate_object) {
    // use placement syntax of operator new to call T's copy constructor
    // with new memory allocated by memory_.Alloc()
    //   (Note: destruction will be taken care of in memory_.Clear())
    T *obj = new (memory_.Alloc()) T(candidate_object);
    collection_.push_back(obj);
    return obj;
  }

  // Make it pseudo compatible with other stl container types.
  T* insert(const T& candidate_object) {
    return Store(candidate_object);
  }

  T* insert(const_iterator iter, const T& candidate_object) {
    return Store(candidate_object);
  }

  // returns the total number of items stored
  inline int size() const { return collection_.size(); }

  // retrieve the n-th item
  inline T *operator[](int index) const {
    ASSERT(index >= 0 && index < size());
    return collection_[index];
  }

  // set/get name for debugging purposes
  inline void set_name(const string& name) {
    name_ = name;
    memory_.set_name(name);
  }
  inline string get_name() const { return name_; }

  inline void Sort(bool (*order_func)(const T *, const T *)) {
    sort(collection_.begin(), collection_.end(), order_func);
  }

  template <typename Compare>
  inline void Sort(const Compare& comp = Compare()) {
    sort(collection_.begin(), collection_.end(), comp);
  }

  inline void DumpToVector(vector<const T *> *result) const {
    result->resize(collection_.size());
    for (int i = 0; i < collection_.size(); i++)
      (*result)[i] = collection_[i];
  }

 private:
  // object collection storage structure
  tCollection collection_;
  MemoryBlock<T, initial_size> memory_;
  string name_;  // name of this collection (for debugging)
};


#endif  // _PUBLIC_UTIL_MEMORY_COLLECTION_H_
