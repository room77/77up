// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

#include "shared_lru_cache.h"

#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif

int main() {
  using namespace std;

  // Test basics and expiration.
  {
    // Create a cache with 2 max size and 1 ms expiration time.
    SharedLRUCache<string, int> cache(2, std::chrono::milliseconds(1));

    cache.insert(make_pair("1", 1));
    ASSERT(cache.find("1") != cache.end());
    // Sleep for 2 ms. Entry should have expired by now.
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    // Entry will be deleted once we try to inspect it (lazy).
    // So unless we inspect, it should be live.
    ASSERT(cache.size() == 1);
    ASSERT(!cache.empty());
    // Let's inspect it (it should expire and return end()).
    ASSERT(cache.find("1") == cache.end());
    // Make sure it has expired.
    ASSERT(cache.size() == 0);
    ASSERT(cache.empty());
  }

  // Test persistence.
  {
    SharedLRUCache<int, int>::iterator it;
    {
      SharedLRUCache<int, int> cache(100);
      // Let's use the operator[] version for a change.
      cache[1] = 10;
      it = cache.find(1);
      ASSERT (it != cache.end());
    }
    // Cache has expired along with all its contents, but iterator is still
    // alive. Unlike STL containers, we should be able to access the object.
    ASSERT(it->first == 1 && it->second == 10);
  }

  // Test operator []
  {
    SharedLRUCache<int, int> cache(100);
    // This should not create anything except a temporary delegate.
    cache[1];
    ASSERT(cache.empty());
    // This should insert <1, 10>.
    cache[1] = 10;
    ASSERT(cache.size() == 1);
    ASSERT(cache[1] == 10);
    cache[1] = 20;
    ASSERT(cache.size() == 1);
    ASSERT(cache[1] == 20);
  }

  // Test LRU
  {
    // Create a cache with 2 max size.
    SharedLRUCache<string, int> cache(2);
    cout << "Adding 1 and 2 in order" << endl;
    cache.insert(make_pair("1", 1));
    cache.insert(make_pair("2", 2));
    cache.DumpContent(cout);
    cout << "Accessing 1, this should bump its position up" << endl;
    cache.find("1");
    cache.DumpContent(cout);
    cout << "Adding 3, so 2 should expire" << endl;
    cache.insert(make_pair("3", 3));
    cache.DumpContent(cout);
    ASSERT(cache.size() == 2);
    ASSERT(cache.find("1") != cache.end());
    ASSERT(cache.find("3") != cache.end());
    cout << "Adding 4 and we last touched 3, so 1 should expire" << endl;
    cache["4"] = 4;
    cache.DumpContent(cout);
    ASSERT(cache.size() == 2);
    ASSERT(cache.find("3") != cache.end());
    ASSERT(cache.find("4") != cache.end());
  }
  return 0;
}
