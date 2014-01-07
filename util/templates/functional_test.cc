/*
  Tests for functional util functions

  @author: Kyle Konrad <kyle@room77.com>
  Copyright 2013, Room 77, Inc.
*/

#include "util/templates/functional.h"

#include "test/cc/unit_test.h"

namespace util {
namespace tl {
namespace test {

struct tBase {
  int a;
  float b;
};

struct tDerived : public tBase {
  string c;
};

TEST(FunctionalTest, TestGet) {
  tBase test({1, 2.0});
  auto getter = Get<tBase, decltype(tBase::a), &tBase::a>();
  EXPECT_EQ(1, getter(test));
}

TEST(FunctionalTest, TestGet_derived) {
  tDerived test;
  test.a = 1;
  test.b = 2.0;
  test.c = "3";
  auto getter = Get<tBase, decltype(tDerived::a), &tDerived::a>();
  EXPECT_EQ(1, getter(test));
  auto getter2 = Get<tDerived, decltype(tDerived::c), &tDerived::c>();
  EXPECT_EQ("3", getter2(test));
}

int Increment(int x) { return x + 1; }
int Square(int x) { return x * x; }

TEST(FunctionalTest, TestCompose_free) {
  EXPECT_EQ(Compose(Square, Increment)(3), 16);
}

TEST(FunctionalTest, TestCompose_lambda) {
  auto increment = [](int x) { return x + 1; };
  auto square = [](int x) { return x * x; };
  EXPECT_EQ(Compose(square, increment)(3), 16);
}

// not supported
// TEST(FunctionalTest, TestCompose_functor) {
//   struct increment {
//     increment() {}
//     increment(increment&&) {}
//     int operator()(int x) const { return x + 1; }
//   };
//   struct square {
//     square() {}
//     square(square&&) {}
//     int operator()(int x) const { return x * x; }
//   };
//   auto increment_and_square_fn = Compose(square(), increment());
//   EXPECT_EQ(increment_and_square_fn(3), 16);
// }

// TEST(FunctionalTest, TestCompose_std) {
//   EXPECT_EQ(Compose(std::negate(), std::negate())(3), 3);
// }


} // namespace test
} // namespace tl
} // namespace util
