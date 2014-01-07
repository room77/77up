// Copyright 2011 B. Uygar Oztekin & Room77, Inc.

// Thread safe LRU cache that implements a useful subset of std::unordered_map
// interface. Optional extra interface is available to specify expiration time.
// Operator[] is supported through a delegate. Unless you assign a value to it,
// nothing is inserted to the cache. Once you get an iterator or delegate,
// it effectively holds a ref counted copy of the data. Iterators never get
// "invalidated" as long as the cache and the iterator are alive. Data is
// reference counted. Even though an entry may be evicted from the cache, the
// actual data may not be deallocated if there is an iterator or delegate to it.
// Most interesting operations take O(1) assuming a good hash function.

#ifndef _PUBLIC_UTIL_CACHE_SHARED_LRU_CACHE_H_
#define _PUBLIC_UTIL_CACHE_SHARED_LRU_CACHE_H_

#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <list>
#include <unordered_map>

#ifdef R77_USE_SERIALIZER
#include "util/serial/serializer.h"
#endif

template<class K, class V, class H = std::hash<K>, class EQ = std::equal_to<K>>
class SharedLRUCache {
  // Convenience typedefs and structs.
  typedef std::shared_ptr<std::pair<K, V>> data_t;
  struct data_hash {
    size_t operator()(const data_t& d) const { return H()(d->first); }
  };
  struct data_eq {
    size_t operator()(const data_t& d1, const data_t& d2) const { return EQ()(d1->first, d2->first); }
  };
  typedef std::list<std::pair<data_t, std::chrono::steady_clock::time_point>> list_t;
  typedef std::unordered_map<data_t, typename list_t::iterator, data_hash, data_eq> map_t;
  typedef std::lock_guard<std::recursive_mutex> lock_t;

 public:
  typedef K                                 key_type;
  typedef typename map_t::size_type         size_type;
  typedef typename data_t::element_type     value_type;
  typedef std::chrono::steady_clock         clock_type;

  // Minimal iterator definition. We only support checking against end().
  struct iterator {
    iterator() {}
    iterator(data_t data) : data_(data) {}
    bool operator == (const iterator& it) const { return data_.get() == it.data_.get(); }
    bool operator != (const iterator& it) const { return data_.get() != it.data_.get(); }
    const value_type& operator*()  const        { return *data_.get(); }
    const value_type* operator->() const        { return data_.get(); }
   private:
    data_t data_;
    friend class SharedLRUCache;
  };
  typedef iterator const_iterator;

  // Allows use of operator [] in most contexts.
  struct delegate {
    delegate(data_t data, SharedLRUCache* cache) : data_(data), cache_(cache) {}
    operator const V&() { return data_->second; }
    void operator=(const V& v) const { data_->second = v; cache_->insert(*data_); }
   private:
    data_t data_;
    SharedLRUCache* cache_;
  };

  SharedLRUCache(size_type limit) : max_size_(limit), expire_(false) {}
  SharedLRUCache(size_type limit, clock_type::duration lifetime)
    : max_size_(limit), lifetime_(lifetime), expire_(true) {}
  virtual ~SharedLRUCache() {}

  // Copy constructor locks both mutexes and copies the remaining data.
  SharedLRUCache(const SharedLRUCache& c) {
    lock_t l(mutex_);
    lock_t cl(c.mutex_);
    max_size_ = c.max_size_;
    map_ = c.map_;
    list_ = c.list_;
    expire_ = c.expire_;
    lifetime_ = c.lifetime_;
  }

  // Unlike unordered_map, insert variants return void for efficiency.
  void insert(const value_type& v, clock_type::time_point tp) {
    data_t data(new value_type(v));
    lock_t l(mutex_);
    auto map_it = map_.find(data);
    if (map_it != map_.end()) {
      list_.erase(map_it->second);
      map_.erase(map_it);
    }
    list_.push_front(typename list_t::value_type(data, tp));
    map_.insert(make_pair(data, list_.begin()));

    while (oversize()) {
      auto it = list_.rbegin();
      map_.erase(it->first);
      list_.pop_back();
    }
  }
  void insert(const value_type& v) {
    insert(v, expire_ ? clock_type::now() + lifetime_ : clock_type::time_point());
  }

  size_type erase(const key_type& k) {
    data_t key(new value_type(k, V()));
    lock_t l(mutex_);
    auto it = map_.find(key);
    if (it == map_.end()) return 0;
    list_.erase(it->second);
    map_.erase(it);
    return 1;
  }
  void erase(const_iterator it) { erase(it->first); }

  iterator find(const key_type& k) const {
    data_t key(new value_type(k, V()));
    lock_t l(mutex_);
    auto it = map_.find(key);
    if (it == map_.end()) return data_t();
    if (expire_ && it->second->second < clock_type::now()) {
      list_.erase(it->second);
      map_.erase(it);
      return data_t();
    }
    list_.push_front(*it->second);
    list_.erase(it->second);
    it->second = list_.begin();
    return it->first;
  }

  iterator  end()      const    { return iterator(data_t()); }
  void      clear()             { lock_t l(mutex_); map_.clear(); list_.clear(); }
  void      rehash(size_type n) { lock_t l(mutex_); map_.rehash(n); }
  bool      empty()    const    { lock_t l(mutex_); return map_.empty(); }
  size_type size()     const    { lock_t l(mutex_); return map_.size(); }
  size_type max_size() const    { lock_t l(mutex_); return max_size_; }

  delegate operator[](const key_type& k) {
    data_t data = find(k).data_;
    if (!data.get()) data.reset(new value_type(k, V()));
    return delegate(data, this);
  }

  // Debug function to dump the contents of the cache.
  std::ostream& DumpContent(std::ostream& out = std::cout,
      const std::string& delim1 = " --> ", const std::string delim2 = "\n",
      const std::string& pre = "{\n", const std::string& post = "}\n") {
    out << pre;
    for (auto it = list_.begin(); it != list_.end(); ++it) {
      #ifdef R77_USE_SERIALIZER
      // Requires serialization library.
      out << serial::Serializer::ToJSON(it->first->first) << delim1
          << serial::Serializer::ToJSON(it->first->second) << delim2;
      #else
      // Alternate way without serialization. May only work with basic types.
      out << it->first->first << delim1 << it->first->second << delim2;
      #endif
    }
    out << post;
    return out;
  }

 protected:
  virtual bool oversize() { return size() > max_size_; }

  size_type max_size_;
  mutable map_t map_;
  mutable list_t list_;
  mutable std::recursive_mutex mutex_;
  mutable clock_type::duration lifetime_;
  mutable bool expire_;
};

#endif  // _PUBLIC_UTIL_CACHE_SHARED_LRU_CACHE_H_
