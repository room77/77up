// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "util/network/util.h"

#include "test/cc/test_main.h"

namespace NetworkUtil {
namespace test {

TEST(Util, Ping) {
  EXPECT_TRUE(Ping("192.168.77.1"));
  EXPECT_TRUE(Ping("www.google.com"));
  EXPECT_TRUE(Ping("www.facebook.com"));
  EXPECT_FALSE(Ping("www.amazon.com"));  // Amazon does not support ping.
  EXPECT_FALSE(Ping("some.nonexistent.site"));
}

}  // namespace test
}  // namespace NetworkUtil
