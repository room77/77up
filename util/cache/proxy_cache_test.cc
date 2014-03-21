#include <thread>
#include "base/common.h"
#include "util/cache/proxy_cache.h"
#include "test/cc/test_main.h"

namespace util {
namespace test {

class ProxyCacheTest : public ::testing::Test {};

TEST_F(ProxyCacheTest, BasicTest) {
  ProxyCache<string, string> proxy_cache(100, chrono::seconds(900));
  string key = "q1";
  EXPECT_EQ("q1q1", proxy_cache(key, [key](const string& old_value){return key + key;}));
  EXPECT_EQ(proxy_cache.hits(), 0);
  EXPECT_EQ(proxy_cache.calls(), 1);
  EXPECT_EQ("q2val", proxy_cache("q2", [](const string& old_value){return "q2val";}));
  EXPECT_EQ(proxy_cache.hits(), 0);
  EXPECT_EQ(proxy_cache.calls(), 2);
  EXPECT_EQ("q1q1", proxy_cache("q1", [](const string& old_value){ return "failed";}));
  EXPECT_EQ(proxy_cache.hits(), 1);
  EXPECT_EQ(proxy_cache.calls(), 3);
  EXPECT_EQ("newVal", proxy_cache(key, [key](const string& old_value){return "newVal";}, false,
    std::chrono::steady_clock::now() + std::chrono::seconds(895)));
  EXPECT_EQ(proxy_cache.hits(), 1);
  EXPECT_EQ(proxy_cache.calls(), 4);
  EXPECT_EQ("newVal", proxy_cache(key, [key](const string& old_value){return "blah";}, false,
    std::chrono::steady_clock::now() + std::chrono::seconds(895+500)));
  EXPECT_EQ(proxy_cache.hits(), 2);
  EXPECT_EQ(proxy_cache.calls(), 5);
}

TEST_F(ProxyCacheTest, AsyncUpdateTest) {
  ProxyCache<string, string> proxy_cache(100, chrono::seconds(86400), chrono::seconds(86400)); // everything is always about to expire
  string key = "a";
  chrono::seconds wait(2);

  EXPECT_EQ("b", proxy_cache(key, [](const string& old_value) { return "b"; }));

  // test that get() is not called when try_no_block is set
  auto start = chrono::steady_clock::now();
  EXPECT_EQ("b", proxy_cache(key, [&](const string& old_value) {
        EXPECT_TRUE(false);
        return "c"; },
      true));
  auto duration = chrono::steady_clock::now() - start;
  EXPECT_LT(duration, wait);

  // spawn thread that will call get()
  bool called = false;
  thread t([&]() {
      auto start = chrono::steady_clock::now();
      EXPECT_EQ("d", proxy_cache(key, [&](const string& old_value) {
            called = true;
            this_thread::sleep_for(wait);
            return "d"; },
          false));
      auto duration = chrono::steady_clock::now() - start;
      EXPECT_TRUE(called);
      EXPECT_GT(duration, wait);
    });

  // check that we can get the old value while the last one is running
  EXPECT_EQ("b", proxy_cache(key, [&](const string& old_value) {
        return "e"; },
      true));

  // wait for the previously spawned thread to finish
  t.join();
  // and check that the value is updated
  EXPECT_EQ("d", proxy_cache(key, [&](const string& old_value) {
        return "f"; },
      true));
}

TEST_F(ProxyCacheTest, MergeTest) {
  ProxyCache<string, vector<int> > proxy_cache(100, chrono::seconds(900), chrono::seconds(90)); // everything is always about to expire
  string key = "a";
  vector<int> v1 = {1, 2};
  vector<int> v2 = {3, 4};
  vector<int> v3 = {1, 2, 3, 4};
  EXPECT_EQ(v1, proxy_cache("a", [v1](const vector<int>& old_value){ return v1; }));
  EXPECT_EQ(v1, proxy_cache("a", [v2](const vector<int>& old_value){
        EXPECT_TRUE(false) << "this should not be called because the value is not old";
        return v2; }));
  EXPECT_EQ(v3, proxy_cache("a", [v2](const vector<int>& old_value){
        vector<int> new_value = old_value;
        new_value.insert(new_value.end(), v2.begin(), v2.end());
        return new_value; }, false,
      chrono::steady_clock::now() + std::chrono::seconds(895)));
}

} // namespace test
} // namespace util
