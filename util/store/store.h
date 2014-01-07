// Copyright 2013 B. Uygar Oztekin

// Copied and modified from Uygar's personal library. Room77 may freely use.
// R77 specific stores are marked copyright Room77. Uygar retains copyright
// of generic infrastructure and stores wrapping free third party libraries.

// In order to be able to open source it in the future, dependency on R77
// codebase is kept to a minimum. If you modify the code, do NOT add extra
// dependencies before talking to Uygar.

#ifndef _PUBLIC_UTIL_STORE_STORE_H_
#define _PUBLIC_UTIL_STORE_STORE_H_

#include <cassert>
#include <memory>
#include <string>
#include "util/factory/factory.h"

///////////////////////////////////////////////////////////////////////////////
//
// Store abstraction provides a unified, stl-like interface over a number of
// data storage options where keys and values are strings. The API is not fully
// stl compatible due to efficiency / practicality.
//
// End user should use the factory mechanism to get a proxy to the store and use
// it via the unified API. This way, underlying storage implementation can be
// replaced without extra effort / code change at any time. For example, you
// start with a local mysql store to store your data and access it. In the
// future your data requirements or restrictions change in a way that you can
// no longer use mysql, but switch to another alternative (e.g. a redis store).
// Hopefully, you won't need to change any of the end user code. You would just
// change the registered store to be redis instead of mysql.
//
// Warning: Due to the limitations in most derived "iterator implementations",
// store iterators have alias semantics. If you copy an iterator, it actually
// shares the same underneat iterator implementation. Hence once one is modified
// the other copy is also modified the same way. To avoid undesirable side
// effects, store iterators should not be copied unless the original copy is
// about to be destroyed. One advantage of this approach is efficiency. It is
// fairly efficient to return or copy store iterators. Even though they are not
// marked as such, due to compactness concerns, all store iterators are const.
// They should not / cannot be used to mutate elements.
//
// Not all stores support all the functionality. Various API components are
// optional. In end-user code, try not to use extra optional functionality that
// not all stores are supposed to support. This maximizes your chances of being
// able to swap a store backend with another when your needs change.
//
// Typical store return types (iterator or result<T>) actually contain extra
// information such as error() or supported(). They can be used to check
// whether or not an operation did or did not succeed for other reasons.
//
// E.g. auto it = store.begin();
//
// - it.error() -> true if there has been another error (e.g. timeout).
// - it.supported() -> true if the operation is implemented.
//   (store does not support this method).
//
// Similarly auto result = store.insert(make_pair("foo", "bar"));
//
// - result.value() / result casted to bool -> true if success, false otherwise.
// - result.error() -> true if there was some sort of error (e.g. timeout).
// - result.supported() -> is the method implemented by this store?
//
// All basic and advanced stores typically support:
// - find()
// - checking an iterator against end()
//
// - size() / empty() are optional (cannot always be implemented efficiently).
//
// Stores that support forward iteration over all elements also support:
// - begin()
// - checking an iterator against any other iterator of the same type.
// - operator ++ on the iterator
//
// Mutable stores, in addition to basic methods, support the following:
// - erase()
// - insert()
//
// - clear() is optional for mutable stores.
//
// In general, if an operation cannot be efficiently implemented by a store,
// it may be preferable not to implement it at all.
//
// If a store does not implement a method, all calls to that method should
// return a result type with the supported bit set to false. Hence, you
// may assert test the methods you depend on once during initialization and
// abort if the methods you rely on are not implemented rather than checking
// this bit on the fly everytime.
//
// error() can be checked if you really care about why an operation has failed.
// if you don't care, the actual return type may be sufficient. Stores should be
// implemented in such a way that, if an error happened, in addition to error()
// bit being set, the actual return type is also sensible (e.g. find() returns
// end(), insert returns false etc.).
//
///////////////////////////////////////////////////////////////////////////////

namespace store {
namespace result {

// Due to a bug in GCC we cannot define these inside the store class.
// If the bug is fixed in the future, we may move them back in.

// Base class for iterator and result<T> to add additional information to
// return types that are kind of optional but good to have for some stores.
// Error can be used to communicate if there has been some unforseen issue
// preventing the operation to be tried (e.g. timeout, server down etc.).
// supported() can be used to communicate that this store does implement this
// particular operation.
class ResultBase {
 public:
  ResultBase() = default;
  ResultBase(bool error, bool supported) : error_(error), supported_(supported) {}
  bool error()     { return error_ || !supported_; }
  bool supported() { return supported_; }

 private:
  bool error_ = false;
  bool supported_ = true;
};

// Result class can be used to send multiple information with the return.
// By default, you want to return bool, iterator, size, etc. but you may also
// want to check if there has been an error, or a specific feature is not
// implemented. Result is automatically castable to type and behaves as the
// type in most cases. In addition, you can also check the following methods:
// - error() -> was there any error preventing the operation (e.g. timeout)?
// - supported() -> true if this store implements this method.
// - value() -> Actual value returned by the operation which can also be
// accessed by auto-casting to type T.
template<class T>
class Result : public ResultBase {
 public:
  Result(T t, bool error = false, bool supported = true) : ResultBase(error, supported), t_(t) {}
  operator const T&() { return t_; }
  const T& value()    { return t_; }
 private:
  T t_;
};
}

template<class Key = std::string, class Data = std::string>
class Store : public Factory<Store<Key, Data>> {
 public:
  virtual ~Store() {}
  using key_type   = Key;
  using data_type  = Data;
  using size_type  = size_t;
  using value_type = std::pair<const key_type, data_type>;
  template<class R> using result = result::Result<R>;

  // Convenience result type to define unimplemented features.
  template<class T>
  struct not_supported : public result<T> {
    not_supported() : result<T>(T(), false, false) {}
  };

  // Derived classes' iterators should inherit from this class directly or
  // indirectly.
  class IteratorBase {
   public:
    virtual ~IteratorBase() {}

    // Mandatory methods:
    virtual bool operator==(const IteratorBase& it) const = 0;
    virtual const value_type& operator*() const = 0;
    virtual const value_type* operator->() const = 0;

    // Optional methods for stores that support iteration:
    virtual result<bool> operator++() { return not_supported<bool>(); }
    virtual result<bool> operator--() { return not_supported<bool>(); }

    // Return a shared_ptr copy of the data.
    virtual std::shared_ptr<const value_type> shared_ptr() const {
      return std::shared_ptr<const value_type>(new value_type(**this));
    }
  };

  // Convenience derived class that uses a share_ptr cache. Some stores'
  // iterator implementation may derive from this instead of IteratorBase.
  class CachingIterator : public IteratorBase {
   public:
    CachingIterator(std::shared_ptr<value_type> cache = nullptr) : cache_(cache) {}
    const value_type& operator*()  const { return *cache_; }
    const value_type* operator->() const { return  cache_.get(); }
    std::shared_ptr<const value_type> shared_ptr() const { return cache_; }

   protected:
    mutable std::shared_ptr<const value_type> cache_;
  };

  // End-user-facing iterator class that uses an CachingIterator instance as
  // underlying implementation for specific derived classes.
  // Iterator is also derives from ResultBase, hence have
  // - error()
  // - supported()
  // - value()
  // methods for compatibility with result type.
  class iterator : public store::result::ResultBase {
   public:
    iterator(IteratorBase* itb, bool error = false, bool supported = true)
      : ResultBase(error, supported), itb_(itb) {};
    bool operator==(const iterator& it) const { return *get() == *it.get(); }
    bool operator!=(const iterator& it) const { return !operator==(it); }
    const value_type& operator*()  const { return get()->operator*(); }
    const value_type* operator->() const { return get()->operator->(); }
    result<bool> operator++() { return get()->operator++(); }
    result<bool> operator--() { return get()->operator++(); }
    const iterator& value() { return *this; }
    std::shared_ptr<const value_type> shared_ptr() const { return get()->shared_ptr(); }

   private:
    IteratorBase* get() const { return itb_.get(); }
    const std::shared_ptr<IteratorBase> itb_;
  };

  struct not_supported_iterator : public iterator {
    not_supported_iterator() : iterator(nullptr, false, false) {}
  };


  /////////////////////////////////////////////////////////////////////////////
  // General methods most stores must support (except write-only stores):
  /////////////////////////////////////////////////////////////////////////////

  // Returns an iterator that can be used to access the key and value. For
  // efficiency, in most implementations, iterator would actually contain a
  // cached copy of the data snapshotted at the time of the find() call.
  virtual iterator find(const key_type& k) const { return not_supported_iterator(); }

  // For basic stores, iterator returned by find() can be checked against end().
  // For stores that support additional functionality such as forward iteration,
  // it can be used to check if an incremented iterator is no longer valid.
  virtual iterator end() const { return not_supported_iterator(); }


  /////////////////////////////////////////////////////////////////////////////
  // Optional: General methods available if they can be implemented efficiently:
  /////////////////////////////////////////////////////////////////////////////

  // Not all stores may efficiently know the number of entries they have.
  // Such stores typically won't implement size() and empty().
  virtual result<size_type> size()  const { return not_supported<size_type>(); }
  virtual result<bool>      empty() const { return not_supported<bool>(); }


  /////////////////////////////////////////////////////////////////////////////
  // Optional: For stores that support lower/upper bound:
  /////////////////////////////////////////////////////////////////////////////

  // Lower bound seeks the first key that is not less than k.
  // Upper bound seeks the first key that is greater than k.
  virtual iterator lower_bound(const key_type& k) const { return not_supported_iterator(); }
  virtual iterator upper_bound(const key_type& k) const { return not_supported_iterator(); }

  /////////////////////////////////////////////////////////////////////////////
  // Optional: Mandatory methods for stores that support forward iteration:
  /////////////////////////////////////////////////////////////////////////////

  // Begin() is only implemented for stores that support forward iteration.
  // By default it returns an "not implemented" iterator.
  virtual iterator begin() const { return not_supported_iterator(); }


  /////////////////////////////////////////////////////////////////////////////
  // Optional: Mandatory methods for mutable stores:
  /////////////////////////////////////////////////////////////////////////////

  // Insert should only succeed if there is no existing element with the same
  // key being inserted. It should return false if a key is already present.
  // If you want to insert or overwrite an existing key, look at update().
  // Clients may use insert's behavior to synchronize and communicate. Only one
  // concurrent insert operation on the same key may succeed, the rest would
  // fail.
  virtual result<bool> insert(const value_type& v) { return not_supported<bool>(); }
  virtual result<bool> update(const value_type& v) { return not_supported<bool>(); }
  virtual result<bool> erase(const key_type& k)    { return not_supported<bool>(); }

  /////////////////////////////////////////////////////////////////////////////
  // Optional: Optional methods for mutable stores:
  /////////////////////////////////////////////////////////////////////////////

  virtual result<bool> clear() { return not_supported<bool>(); }
};

}

#endif  // _PUBLIC_UTIL_STORE_STORE_H_
