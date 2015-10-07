// Copyright 2014 Room77 Inc. All Rights Reserved.
// Author: kyle@room77.com (Kyle Konrad)

#include "util/templates/comparator.h"
#include "test/cc/test_main.h"

namespace util {
namespace tl {
namespace test {

struct tTest {
  int a;
  string b;
};

struct tDerived : public tTest {
  float c;
};

TEST(ComparatorTest, TestLessMember) {
  tTest x = {1, "a"};
  tTest y = {2, "b"};
  auto comp = less_member<tTest, decltype(tTest::a), &tTest::a>();
  EXPECT_TRUE(comp(x,y));
}

TEST(ComparatorTest, TestLessMember_derived) {
  tDerived x;
  x.a = 1;
  x.b = "a";
  x.c = 1.0;
  tDerived y;
  y.a = 2;
  y.b = "a";
  y.c = 2.0;
  // The first template parameter must be the class in which in the member was defined
  auto comp = less_member<tTest, decltype(tDerived::a), &tDerived::a>();
  EXPECT_TRUE(comp(x,y));
}

} // namespace test
} // namespace tl
} // namespace util
