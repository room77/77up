// Copyright 2013  Room77, Inc.

// AutoProxyCache is built on top of ProxyCache
// it automatically refreshes after every query

#ifndef _PUBLIC_UTIL_CACHE_AUTO_PROXY_CACHE_H_
#define _PUBLIC_UTIL_CACHE_AUTO_PROXY_CACHE_H_

#include "base/common.h"
#include "util/cache/proxy_cache.h"
#include "util/thread/thread_pool.h"

namespace util {

template<class K, class V>
class AutoProxyCache {
 public:
  AutoProxyCache(size_t size, chrono::steady_clock::duration ttl, int num_threads) :
      proxy_cache_(size, ttl, ttl*2), // refreshThreshold >  ttl so everything is always "about to expire"
      thread_pool_(num_threads) {}

  V operator()(const K key, function<V(const V&)> get,
               chrono::steady_clock::time_point now=chrono::steady_clock::now()) {
    thread_pool_.Add([=]() { this->proxy_cache_(key, get, false, now); });
    V value = proxy_cache_(key, get, true, now);
    return value;
  }


private:
  ProxyCache<K, V> proxy_cache_;
  threading::ThreadPool thread_pool_;
};

} // namespace util

#endif  // _PUBLIC_UTIL_CACHE_AUTO_PROXY_CACHE_H_
