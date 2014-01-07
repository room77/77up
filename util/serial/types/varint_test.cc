// Copyright 2013 Room77, Inc.
// Author: Uygar Oztekin

#include "util/serial/types/varint.h"

#include <iostream>
#include <sstream>

#include "base/common.h"

int Value(int i) { return i; }

// Print the data pattern in hex. 2 chars per byte.
string Print(const string& input) {
  stringstream ss;
  ss << setfill('0');
  for (uint8_t c : input) {
    uint16_t i = c;
    ss << hex << setw(2) << i;
  }
  return ss.str();
}

template<class T, bool is_fixed = false>
void Test(T input) {
  stringstream ss;
  varint<T, is_fixed>(input).ToBinary(ss);
  cout << "Type: " << typeid(T).name() << " Value: " << setw(20) << input
       << " Size: " << ss.str().size()
       << " Hex: " << Print(ss.str()) << endl;

  if (is_fixed) ASSERT(ss.str().size() == sizeof(T));
  ASSERT(ss.good());
  varint<T, is_fixed> temp;
  ASSERT(temp.FromBinary(ss));
  ASSERT_EQ(temp, input);
}

int init_main() {
  varint<int> i = 5;
  varint<int> j = 6;
  ASSERT(i < j);
  ASSERT(i < 10);
  ASSERT(5 < j);
  ASSERT(i == Value(i));

  Test(uint32_t(1));
  Test(uint32_t(150));
  Test(uint32_t(65535));
  Test(uint32_t(123456789));

  Test(int32_t(1));
  Test(int32_t(-1));
  Test(int32_t(75));
  Test(int32_t(-75));
  Test(int32_t(65535));
  Test(int32_t(-65535));

  Test(10);
  Test(-10);
  Test(63);
  Test(-64);
  Test(64);
  Test(-65);

  Test(uint8_t('a'));
  Test(int8_t('a'));

  Test(uint32_t(1UL<<31));
  Test(uint64_t(1UL<<31));
  Test(uint64_t(1UL<<63));

  uint64_t now = chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count();
  Test(now);

  // Test fixed types.
  Test<uint64_t, true>(uint64_t(0));
  Test<uint64_t, true>(uint64_t(1UL<<1));
  Test<uint64_t, true>(uint64_t(1UL<<6));
  Test<uint64_t, true>(uint64_t(1UL<<7));
  Test<uint64_t, true>(uint64_t(1UL<<16));
  Test<uint64_t, true>(uint64_t(1UL<<32));
  Test<uint64_t, true>(uint64_t(1UL<<63));

  return 0;
}
