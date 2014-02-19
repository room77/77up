// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

//
// HttpClient and HttpsClient implement http GET/POST commands
// They are both instances of HttpBase
//

#ifndef _PUBLIC_UTIL_NETWORK_CACHED_HTTPCLIENT_H_
#define _PUBLIC_UTIL_NETWORK_CACHED_HTTPCLIENT_H_

#include <iomanip>
#include <thread>
#include "util/network/httpclient.h"
#include "util/network/netclient.h"
#include "util/serial/serializer.h"
#include "util/cache/shared_lru_cache.h"

template <class NetBase, int cache_lifetime_sec, int cache_size, int poll_freq_ms>
class CachedHttpBase : public HttpBase<NetBase> {
  typedef std::lock_guard<std::recursive_mutex> lock_t;
 public:
  struct CacheKey {
    bool is_get;
    string host;
    int port;
    string path;
    string message;

    // Let's use the same struct as the "hash functor" as well.
    size_t operator()(const CacheKey& k) { return std::hash<string>()(ToBinary()); }

    // Operator == for unordered_map.
    bool operator==(const CacheKey& k) const {
      return is_get == k.is_get && host == k.host && port == k.port &&
             path == k.path && message == k.message;
    }
    SERIALIZE(is_get*1 / host*2 / port*3 / path*4 / message*5);
  };

  struct CacheEntry {
    CacheEntry() : ready(false), success(false), status_code(-1) {}
    bool ready;
    bool success;
    int status_code;
    string reply;
  };
  typedef shared_ptr<CacheEntry> CacheValue;

  bool HttpGet(const string& host, int port, const string& path,
               int *status_code, string *reply) {
    CacheKey key = { true, host, port, path, "" };
    bool own;
    auto it = GetIterator(key, &own);

    if (!own) {
      for(int i = 0; !it->second->ready && i < this->timeout_; i += poll_freq_ms) {
        this_thread::sleep_for(chrono::milliseconds(poll_freq_ms));
      }
      if (!it->second->ready) return false;
      *status_code = it->second->status_code;
      *reply = it->second->reply;
    }
    else {
      it->second->success = HttpBase<NetBase>::HttpGet(host, port, path, status_code, reply, nullptr);
      it->second->status_code = *status_code;
      it->second->reply = *reply;
      it->second->ready = true;
      if (!it->second->success) Cache().erase(key);
    }
    return it->second->success;
  }


  bool HttpPost(const string& host, int port, const string& path,
                const string& message, int *status_code, string *reply) {
    CacheKey key = { false, host, port, path, message };
    bool own;
    auto it = GetIterator(key, &own);

    if (!own) {
      for(int i = 0; !it->second->ready && i < this->timeout_; i += poll_freq_ms) {
        this_thread::sleep_for(chrono::milliseconds(poll_freq_ms));
      }
      if (!it->second->ready) {
        return false;
      }
      *status_code = it->second->status_code;
      *reply = it->second->reply;
    }
    else {
      it->second->success = HttpBase<NetBase>::HttpPost(host, port, path, message, status_code, reply, nullptr);
      it->second->status_code = *status_code;
      it->second->reply = *reply;
      it->second->ready = true;
      if (!it->second->success) Cache().erase(key);
    }
    return it->second->success;
  }

  static void DumpStats(ostream& out = cout) {
    out << "Requests received : " << total() << endl;
    out << "Requests sent out : " << live() << endl;
    out << "Cache hit ratio   : " << setprecision(4)
        << (total() - live()) * 100.0 / total() << '%' << endl;
  }

 protected:
   typename SharedLRUCache<CacheKey, CacheValue, CacheKey>::iterator
      GetIterator(const CacheKey& key, bool* own) {
    lock_t lock(mutex());
    auto it = Cache().find(key);
    *own = it == Cache().end();
    ++total();
    if (*own) {
      ++live();
      CacheValue value(new CacheEntry());
      Cache().insert(make_pair(key, value));
      it = Cache().find(key);
      ASSERT(it != Cache().end());
    }
    return it;
  }

  // Consider this as cache_. Trick for thread safe initialization of shared cache.
  static SharedLRUCache<CacheKey, CacheValue, CacheKey>& Cache() {
    static SharedLRUCache<CacheKey, CacheValue, CacheKey>
        cache_(cache_size, chrono::seconds(cache_lifetime_sec));
    return cache_;
  }

  static std::recursive_mutex& mutex() {
    static std::recursive_mutex mutex;
    return mutex;
  }

  static long& live() { static long live = 0; return live; }
  static long& total() { static long total = 0; return total; }

};

template<int cache_lifetime_sec = 45, int cache_size = 500, int poll_freq_ms = 100>
class CachedHttpClient
  : public CachedHttpBase<NetClient, cache_lifetime_sec, cache_size, poll_freq_ms> {};

template<int cache_lifetime_sec = 45, int cache_size = 500, int poll_freq_ms = 100>
class CachedHttpsClient
  : public CachedHttpBase<SSLClient, cache_lifetime_sec, cache_size, poll_freq_ms> {};

#endif  // _PUBLIC_UTIL_NETWORK_CACHED_HTTPCLIENT_H_
