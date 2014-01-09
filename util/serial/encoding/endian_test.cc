// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/encoding/endian.h"

#include "test/cc/test_main.h"

namespace serial {
namespace endian {
namespace test {

TEST(Endian, IsBigEndian) {
  EXPECT_FALSE(IsBigEndian());
}

TEST(Endian, SwapEndianness) {
  {
    uint16_t convert = 0x0102;
    SwapEndianness(&convert);
    EXPECT_EQ(0x0201, convert);
  }
  {
    int convert = 0x0102;
    SwapEndianness(&convert);
    EXPECT_EQ(0x02010000, convert);
  }
  {
    int convert = 0x01020304;
    SwapEndianness(&convert);
    EXPECT_EQ(0x04030201, convert);
  }
  {
    uint64_t convert = 0x0102030405060708;
    SwapEndianness(&convert);
    EXPECT_EQ(0x0807060504030201, convert);
  }
}

}  // namespace test
}  // namespace endian
}  // namespace serial
