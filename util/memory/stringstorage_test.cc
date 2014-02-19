
#include <limits>
#include "util/memory/stringstorage.h"

#include "util/string/strutil.h"
#include "test/cc/test_main.h"
#include "util/thread/thread_pool.h"
#include "util/init/init.h"

extern bool gFlag_stringstorage_lock;
INIT_ADD_REQUIRED("test", 90, []{ gFlag_stringstorage_lock = false; });

namespace test {

TEST(StringStorage, Sanity) {
  string s = "hello";

  const char *s1 = gStringStorage.Store("hello");
  const char *s2 = gStringStorage.Store("world");
  const char *s3 = gStringStorage.Store("travel");
  const char *s4 = gStringStorage.Store("worldmap", 5);  // "world"
  const char *s5 = gStringStorage.Store("california");
  const char *s6 = gStringStorage.Store(s);  // "hello"

  EXPECT_TRUE(s1 != s2);  // different words
  EXPECT_TRUE(s1 == s6);  // both are "hello"
  EXPECT_TRUE(s2 == s4);  // both are "world"
  EXPECT_TRUE(!strcmp(s.c_str(), s1));  // both are "hello"
  EXPECT_TRUE(!strcmp("travel", s3));  // both are "travel"
  EXPECT_TRUE(!strcmp("california", s5));  // both are "california"
}

TEST(StringStorage, MultiThreaded) {
  std::function<void(const string& base)> func = [](const string& base) {
        string s = base + "hello";

        const char *s1 = gStringStorage.Store(base + "hello");
        const char *s2 = gStringStorage.Store(base + "world");
        const char *s3 = gStringStorage.Store(base + "travel");
        const char *s4 = gStringStorage.Store((base + "worldmap").c_str(),
                                              base.size() + 5);  // "world"
        const char *s5 = gStringStorage.Store(base + "california");
        const char *s6 = gStringStorage.Store(s);  // "hello"

        // LOG(INFO) << string(s2) << ", " << string(s4);

        EXPECT_NE(s1, s2);  // different words
        EXPECT_EQ(s1, s6);  // both are "hello"
        EXPECT_EQ(string(s2), string(s4));  // both are "world"
        EXPECT_EQ(s2, s4);  // both are "world"
        EXPECT_EQ(s, string(s1));  // both are "hello"
        EXPECT_EQ(base + "travel", string(s3));  // both are "travel"
        EXPECT_EQ(base + "california", string(s5));  // both are "california"
      };

  ::util::threading::ThreadPool pool(512);
  for (int i = 0; i < 4096; ++i)
    pool.Add(std::bind(func, strutil::ToString(i)));

  pool.Wait();
}

}  // namespace test

