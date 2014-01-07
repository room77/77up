// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

#include <iostream>
#include <cstdio>
#include <chrono>

#include "util/init/main.h"
#include "util/cache/group_cache.h"

using namespace std;

string tmpdir = "/tmp/group_map_test.XXXXXXXX";

void DumpStats() {
  cout << "Current contents of the directory: " << endl;
  system(("find " + tmpdir).c_str());
  cout << "Number of bytes in each file" << endl;
  system(("wc -c " + tmpdir + "/*").c_str());
}

void Cleanup(void) {
  DumpStats();
  cout << "Cleaning up " << tmpdir << endl;
  system(("rm -rf " + tmpdir).c_str());
}

// A simple grouper for test purposes. Group keys by length of the key.
struct KeyToGroup  {
  string operator()(const string& key) {
    stringstream ss;
    ss << key.size();
    return ss.str();
  }
};

extern int gFlag_loglevel;

void DumpContents(const string& file) {
  system(("cat " + file).c_str());
}

int init_main() {
  gFlag_loglevel = 0; //3;    // Override debug level to output debug info.
  int max_num = 1000;

  // Test binary input output
  stringstream ss;
  for (int i = 0; i < 10; ++i) {
    stringstream num;
    num << i;
    GroupCache::value_type v;
    v.key = "key" + num.str();
    *v.data = "val" + num.str();
    v.time = GroupCache::Now();
    v.Write(ss);
  }

  for (int i = 0; i < max_num; ++i) {
    GroupCache::value_type v;
    if (!v.Read(ss)) break;
    cout << v.key << " " << *v.data << " " << v.time << endl;
  }

  ASSERT(mkdtemp(&tmpdir[0]));
  cout << "Temporary directory created: " << tmpdir << endl;
  atexit(Cleanup);

  // Test basic functionality and JSON input / output
  GroupCache::Policy policy;
  policy.max_cache_size = 1;
  policy.max_life = 2;
  policy.num_local_shards = 4;
  GroupCache cache(tmpdir, GroupCache::Grouper(KeyToGroup()), policy);

  // Insert max_num entries, creating 3 groups:
  // - "4" numbers 0 to 9
  // - "5" numbers 10 to 99
  // - "6" numbers max_num to 999
  for (int i = 0; i < max_num; ++i) {
    stringstream num;
    num << i;
    auto p = cache.insert("key" + num.str(), "val" + num.str());
    ASSERT(p.second);
    ASSERT(p.first != cache.end());
    ASSERT(p.first->key == "key" + num.str());
    ASSERT(*p.first->data == "val" + num.str());
  }

  // Make sure the max_num items can be fetched.
  for (int i = 0; i < max_num; ++i) {
    stringstream num;
    num << i;
    auto it = cache.find("key" + num.str());
    ASSERT(it != cache.end());
    ASSERT(it->key == "key" + num.str());
    ASSERT(*it->data == "val" + num.str());
  }

  ASSERT(cache.find("does-not-exist") == cache.end());

  // Erase the first 5 items.
  for (int i = 0; i < 5; ++i) {
    stringstream num;
    num << i;
    cache.erase("key" + num.str());
  }

  // Flush all and clear the cache.
  LOG(INFO) << "Number of items in cache: " << cache.size();
  cache.clear();
  LOG(INFO) << "Number of items in cache: " << cache.size();

  // Fetch them again.
  for (int i = 0; i < max_num; ++i) {
    stringstream num;
    num << i;
    auto it = cache.find("key" + num.str());
    if (i < 5) ASSERT(it == cache.end());
    else {
      ASSERT(it != cache.end());
      ASSERT(it->key == "key" + num.str());
      ASSERT(*it->data == "val" + num.str());
    }
  }

  // Wait 1 seconds and make sure we still have the items.
  sleep(1);

  // Fetch them again.
  for (int i = 0; i < max_num; ++i) {
    stringstream num;
    num << i;
    auto it = cache.find("key" + num.str());
    if (i < 5) ASSERT(it == cache.end());
    else {
      ASSERT(it != cache.end());
      ASSERT(it->key == "key" + num.str());
      ASSERT(*it->data == "val" + num.str());
    }
  }

  cout << "Before we expire stuff:" << endl;
  DumpStats();
  // DumpContents(tmpdir + "/4");
  // DumpContents(tmpdir + "/5");
  // DumpContents(tmpdir + "/6");


  // Wait Another second and make sure all items are gone.
  sleep(1);
  for (int i = 0; i < max_num; ++i) {
    stringstream num;
    num << i;
    auto it = cache.find("key" + num.str());
    ASSERT(it == cache.end());
  }

  cache.DumpStats();

  cout << "\nTest passed. About to exit and dump current directory structure.\n" << endl;
  cout << "Since we expired everything, directory should be empty: " << endl;
  return 0;
}
