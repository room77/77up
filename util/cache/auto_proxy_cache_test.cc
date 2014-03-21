#include "base/common.h"
#include "util/cache/auto_proxy_cache.h"
#include "test/cc/test_main.h"

namespace util {
namespace test {

TEST(AutoProxyCache, BasicTest) {
  AutoProxyCache<string, string> cache(100, chrono::seconds(86400), 4);
  string key = "a";

  // first request may call get() either once or twice
  EXPECT_EQ("b", cache(key, [&](const string& old_value) { return "b"; }));


  // new request to update that should return immediately and call get() asynchronously
  // the asynchronous call may finish before the lookup
  EXPECT_IN(cache(key, [&](const string& old_value) { return "c"; }), {"b", "c"});

  // check that asynchonrous update was successful
  string value;
  while ((value = cache(key, [&](const string& old_value) { return "d"; })) == "b") {}

  LOG(INFO) << value;
  EXPECT_IN(value, {"c", "d"});
}

} // namespace test
} // namespace util
