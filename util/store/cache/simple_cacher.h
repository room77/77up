// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_CACHE_SIMPLE_CACHER_H_
#define _PUBLIC_UTIL_STORE_CACHE_SIMPLE_CACHER_H_

#include <cassert>
#include <memory>
#include <thread>
#include <limits>
#include <string>
#include <list>
#include <unordered_map>
#include <iostream>
#include "../store.h"

///////////////////////////////////////////////////////////////////////////////
//
// This is a simple store cacher that sits on top of an undelying store and
// caches the lookups. We have two modes of operation:
//
// 1) Cache all: Everything is loaded to ram upon construction and the data is
//    accessed in a read-only way from then on without mutexes and with minimal
//    overhead. It is pretty much similar to an unordered_map lookup.
//
// 2) Cache some: Nothing is loaded in memory in constructor. Everything is
//    loaded on demand. We have a fixed cache size that caches successful as
//    well as unsuccessful lookups. Hence, you should allocate enough cache
//    size (in number of elements) to account for unsuccessful lookups as well.
//    Cache implementation is LRU. There is no timeout.
//
// This class can be used to have a store that loads everything in memory in
// production and can load stuff on demand on dev if store registration detects
// the environment and sets the preload_all parameter accordingly.
//
// Optimized for prodution use case for minimal efficiency and memory overhead.
//
///////////////////////////////////////////////////////////////////////////////

namespace store {

template<class Key = std::string, class Data = std::string>
class SimpleCacher : public Store<Key, Data> {
 public:
  using Parent       = Store<Key, Data>;
  using Child        = Store<Key, Data>;
  using key_type     = typename Parent::key_type;
  using data_type    = typename Parent::data_type;
  using size_type    = typename Parent::size_type;
  using value_type   = typename Parent::value_type;
  using iterator     = typename Parent::iterator;
  template<class R> using result = result::Result<R>;

 protected:
  // Cache related definitions.
  using data_t = std::shared_ptr<const value_type>;
  using list_t = std::list<data_t>;
  struct meta_data_t {
    typename list_t::iterator list_iter;
    bool end = true;
  };
  struct data_hash_eq {
    size_t operator()(const data_t& d) const { return std::hash<key_type>()(d->first); }
    bool operator()(const data_t& d1, const data_t& d2) const { return d1->first == d2->first; }
  };
  using cache_t = std::unordered_map<data_t, meta_data_t, data_hash_eq, data_hash_eq>;
  using lock_t = std::lock_guard<std::recursive_mutex>;

 public:
  class SimpleCacherIterator : public Parent::CachingIterator {
   public:
    SimpleCacherIterator() : is_end_(true) {}
    SimpleCacherIterator(data_t data) : is_end_(false) { this->cache_ = data; }
    virtual bool operator==(const typename Parent::IteratorBase& it) const {
      auto& rhs = dynamic_cast<const SimpleCacherIterator&>(it);
      return (is_end_ || rhs.is_end_) ? is_end_ && rhs.is_end_ : *this->cache_ == *rhs.cache_;
    }
   private:
    bool is_end_;
  };

  // Construct using a mutable shared proxy of the underlying store.
  SimpleCacher(typename Child::shared_proxy store, bool preload_all = false, int cache_size = std::numeric_limits<int>::max())
     : store_(store), preload_all_(preload_all), cache_size_(cache_size) {
    if (fail()) return;
    if (preload_all_) {
      for (auto it = store_->begin(); it != store_->end(); ++it) {
        cache_.insert(std::make_pair(it.shared_ptr(), meta_data_t()));
      }
    }
  }

  virtual iterator          end()   const { return iterator(new SimpleCacherIterator()); }
  virtual result<bool>      empty() const { return store_->empty(); }
  virtual result<size_type> size()  const { return store_->size(); }

  virtual iterator find(const key_type& k) const {
    // build a cache key from k.
    data_t key;
    key.reset(new value_type{k, Data()});
    // If we are in read all in memory mode, do not lock the mutex.
    if (preload_all_) {
      auto it = cache_.find(key);
      return it == cache_.end() ? end() : iterator(new SimpleCacherIterator(it->first));
    } else {
      // We are in "cache a subset" mode, need to lock mutex.
      lock_t l(mutex_);
      auto it = cache_.find(key);
      if (it != cache_.end()) {
        // If it is in the cache, move the item to the front of the queue.
        list_.push_front(*it->second.list_iter);
        list_.erase(it->second.list_iter);
        it->second.list_iter = list_.begin();
        return it->second.end ? end() : iterator(new SimpleCacherIterator(it->first));
      } else {
        // We don't have it in cache, find it from the store.
        // Unlock the mutex while we are doing the lookup.
        mutex_.unlock();

        auto jt = store_->find(k);
        meta_data_t meta_data;
        meta_data.end = jt == store_->end();
        data_t data = meta_data.end ? key : jt.shared_ptr();

        // Remaining operations need to be guarded. Lock the mutex again.
        mutex_.lock();

        list_.push_front(data);
        meta_data.list_iter = list_.begin();
        cache_.insert(make_pair(data, meta_data));

        // If we hit the limit, remove old entries.
        if (cache_.size() > cache_size_ ) {
          cache_.erase(*list_.rbegin());
          list_.pop_back();
        }
        return meta_data.end ? end() : iterator(new SimpleCacherIterator(data));
      }
    }
  }

  bool fail() const { return store_.get() == nullptr; }

 private:
  typename Child::shared_proxy store_;
  bool preload_all_;
  int cache_size_;
  mutable list_t list_;
  mutable cache_t cache_;
  mutable std::recursive_mutex mutex_;
};

}

#endif  // _PUBLIC_UTIL_STORE_CACHE_SIMPLE_CACHER_H_
