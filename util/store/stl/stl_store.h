// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_STL_STL_STORE_H_
#define _PUBLIC_UTIL_STORE_STL_STL_STORE_H_

///////////////////////////////////////////////////////////////////////////////
//
// This store is a mutable store that supports a wide range of functionality.
// It can wrap an stl map or unordered_map with arbitrary key and value types.
//
// For efficiency, not all operations are made thread safe.
//
// The following operations are safe to be used across multiple threads if usage
// is strictly limited to these:
// - find()
// - insert()
// - erase()
// - clear()
//
// Rest of the methods are not thread safe if the store is modified. They can
// only be used safely if the store is constructed once and not modified during
// concurrent access. In short, read-only access is thread safe.
//
///////////////////////////////////////////////////////////////////////////////

#include <mutex>
#include "../store.h"

namespace store {

template<class T>
class StlStore : public Store<typename T::key_type, typename T::mapped_type> {
  using Parent       = Store<typename T::key_type, typename T::mapped_type>;
  using key_type     = typename Parent::key_type;
  using data_type    = typename Parent::data_type;
  using size_type    = typename Parent::size_type;
  using value_type   = typename Parent::value_type;
  using iterator     = typename Parent::iterator;
  template<class R> using result = result::Result<R>;

 protected:
  class StlStoreIterator : public Parent::IteratorBase {
   public:
    StlStoreIterator(const T& m, typename T::iterator it ) : map_(m), it_(it) { }
    virtual bool operator==(const typename Parent::IteratorBase& it) const {
      const StlStoreIterator& rhs = dynamic_cast<const StlStoreIterator&>(it);
      return it_ == rhs.it_;
    }
    result<bool> operator++() { ++it_; return it_ != map_.end(); }

    const value_type& operator*() const { return *it_; }
    const value_type* operator->() const { return &*it_; }

   private:
    const T& map_;
    typename T::iterator it_;
  };

 public:
  StlStore() = default;
  StlStore(T& m) { map_.swap(m); }
  template<class C>
  StlStore(const C& container) { for (auto& kv : container) insert(kv); }

  iterator find(const key_type& k) const {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    return iterator(new StlStoreIterator(map_, map_.find(k)));
  }

  result<size_type> size() const { return map_.size(); }
  result<bool> empty() const { return map_.empty(); }

  iterator begin() const { return iterator(new StlStoreIterator(map_, map_.begin())); }
  iterator end() const { return iterator(new StlStoreIterator(map_, map_.end())); }

  iterator lower_bound(const key_type& k) const { return lower_impl(k, has_lower_bound(&map_)); }
  iterator upper_bound(const key_type& k) const { return upper_impl(k, has_lower_bound(&map_)); }

  result<bool> insert(const value_type& v) {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    auto p = map_.insert(v);
    return p.second;
  }

  result<bool> erase(const key_type& k) {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    return map_.erase(k);
  }

  result<bool> clear() {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    map_.clear();
    return true;
  }

 protected:
  // Define lower / upper bound if the map container type T has lower_bound
  // implemented. has_lower_bound returns a bool if implemented, int otherwise.
  // We use this to differentiate within the implementation methods.
  template<class M>
  static constexpr auto has_lower_bound(const M*) -> decltype(static_cast<M*>(0)->lower_bound(key_type()), bool()) { return true; }
  static constexpr int  has_lower_bound(...) { return -1; }

  iterator lower_impl(const key_type& k, bool) const { return iterator(new StlStoreIterator(map_, map_.lower_bound(k))); }
  iterator upper_impl(const key_type& k, bool) const { return iterator(new StlStoreIterator(map_, map_.upper_bound(k))); }
  iterator lower_impl(const key_type& k, int) const  { return typename Parent::not_supported_iterator(); }
  iterator upper_impl(const key_type& k, int) const  { return typename Parent::not_supported_iterator(); }

  mutable T map_;
  mutable std::recursive_mutex mutex_;
};

}

#endif  // _PUBLIC_UTIL_STORE_STL_STL_STORE_H_
