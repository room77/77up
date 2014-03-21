// Copyright 2013  Room77, Inc.

// ProxyCache is built on top of SharedLRUCache.
// it's designed to avoid "thundering herd" problem
// () operator retrieves the value from the cache or
// recomputes it using suplied get() function in case of cache miss
// or if the value in the cache is stale

#ifndef _PUBLIC_UTIL_CACHE_PROXY_CACHE_H_
#define _PUBLIC_UTIL_CACHE_PROXY_CACHE_H_

#include <condition_variable>
#include "base/common.h"
#include "util/cache/shared_lru_cache.h"

namespace util {

template<class K, class V>
class ProxyCache {
 public:
  ProxyCache(size_t size, chrono::steady_clock::duration ttl,
             chrono::steady_clock::duration refreshThreshold=
             chrono::steady_clock::duration::max()) :
      cache_(size),
      ttl_(ttl),
      refreshThreshold_(refreshThreshold == chrono::steady_clock::duration::max() ?
                        ttl / 100 : refreshThreshold) {}

  size_t hits()  const {return stats_.hits;}
  size_t calls() const {return stats_.calls;}

  // argument to get is the old value in this entry
  // if this is a new entry, the argument is a default constructed value
  V operator() (const K key, function<V(const V&)> get,
                bool try_no_block=false, /* return immediately on hit even if entry will expire soon */
                chrono::steady_clock::time_point now=chrono::steady_clock::now()) {
    ++stats_.calls;
    value_type v;
    while (true) {
      unique_lock<mutex> lock(mutex_);
      v = cache_[key];
      if (v.expire == chrono::steady_clock::time_point() ||
          now > v.expire) {                         // miss || expired
        if (!v.inProgress) {                        // need to run query ourself
          v.inProgress = true;
          cache_[key] = v;
          break;
        } else {                                    // wait
          cond_var_.wait(lock);
        }
      } else {                                      // hit
        if (now < v.expire - refreshThreshold_      // will not expire soon
            || v.inProgress                         // or someone is working on refresh
            || try_no_block) {                      // or we don't want to refresh
          ++stats_.hits;
          return v.value;
        } else {                                    // will expire soon and not being refreshed. need to run query ourself
          v.inProgress = true;
          cache_[key] = v;
          break;
        }
      }
    }
    // run query
    v.expire = now + ttl_;
    v.value = get(v.value);
    v.inProgress = false;

    // save result
    unique_lock<mutex> lock(mutex_);
    cache_[key] = v;
    cond_var_.notify_all();
    return v.value;
  }

 private:
  struct value_type {
    chrono::steady_clock::time_point expire = chrono::steady_clock::time_point();
    volatile bool inProgress = false;
    V value;
  };

  struct {
    size_t hits = 0;
    size_t calls = 0;
  } stats_;

  mutex mutex_;
  SharedLRUCache<K, value_type> cache_;
  condition_variable cond_var_;
  chrono::steady_clock::duration ttl_;
  chrono::steady_clock::duration refreshThreshold_;
};

} // namespace util

#endif  // _PUBLIC_UTIL_CACHE_PROXY_CACHE_H_
