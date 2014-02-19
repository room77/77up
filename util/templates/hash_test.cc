// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/templates/hash.h"

#include "test/cc/test_main.h"

namespace util {
namespace tl {
namespace test {

struct Base {
  int first = 1;
  char second = 'S';
  string third = "third";
};

struct Derived : public Base {
  string fourth = "fourth";
};

TEST(hash_member, Sanity) {
  {
    size_t expected = std::hash<int>()(1);
    size_t actual = hash_member<Base, int, &Base::first>()(Base());
    EXPECT_EQ(expected, actual);
  }
  {
    size_t expected = std::hash<char>()('S');
    size_t actual = hash_member<Base, char, &Base::second>()(Base());
    EXPECT_EQ(expected, actual);
  }
  {
    size_t expected = std::hash<string>()("third");
    size_t actual = hash_member<Base, string, &Base::third>()(Base());
    EXPECT_EQ(expected, actual);
  }
}

TEST(hash_member_in_base, Sanity) {
  {
    size_t expected = std::hash<int>()(1);
    size_t actual = hash_member<Base, int, &Derived::first>()(Derived());
    EXPECT_EQ(expected, actual);
  }
  {
    size_t expected = std::hash<char>()('S');
    size_t actual = hash_member<Base, char, &Derived::second>()(Derived());
    EXPECT_EQ(expected, actual);
  }
  {
    size_t expected = std::hash<string>()("third");
    size_t actual = hash_member<Base, string, &Derived::third>()(Derived());
    EXPECT_EQ(expected, actual);
  }
  {
    size_t expected = std::hash<string>()("fourth");
    size_t actual = hash_member<Derived, string, &Derived::fourth>()(Derived());
    EXPECT_EQ(expected, actual);
  }
}

}  // namespace test
}  // namespace tl
}  // namespace util
