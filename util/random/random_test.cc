// Copyright 2013 Room77, Inc.
// Author: akearney@room77.com (Andy Kearney), pramodg@room77.com (Pramod Gupta)

#include "util/random/random.h"
#include "test/cc/test_main.h"

namespace util {
namespace random {
namespace test {

template<typename T>
class RandomTest : public testing::Test {
 public:
  virtual ~RandomTest() {}

 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}

  int MaxAllowedCollisions() const {
    return 2 / sizeof(T);
  }
};

// Note: int_8 is too small to do any significant tests.
typedef testing::Types<uint8_t, uint16_t, uint32_t, uint64_t, int16_t, int32_t,
    int64_t> IntegralTypes;
TYPED_TEST_CASE(RandomTest, IntegralTypes);

TYPED_TEST(RandomTest, GetRandomNumber) {
  int num_collisions = 0;
  for (int i = 0; i < 100; ++i)
    num_collisions += (GetRandomNumber<TypeParam>() == GetRandomNumber<TypeParam>());

  EXPECT_GE(this->MaxAllowedCollisions(), num_collisions);
}

TYPED_TEST(RandomTest, GetRandomNumberInRange) {
  int num_collisions = 0;
  for (int i = 0; i < 100; ++i) {
    TypeParam n1 = GetRandomNumberInRange<TypeParam, 5, 105>();
    TypeParam n2 = GetRandomNumberInRange<TypeParam, 5, 105>();
    num_collisions += (n1 == n2);
    EXPECT_TRUE(5 <= n1 && n1 <= 105);
    EXPECT_TRUE(5 <= n2 && n2 <= 105);
  }
  EXPECT_GE(5, num_collisions);
}

TYPED_TEST(RandomTest, GetUniqueHash) {
  int num_collisions = 0;
  for (int i = 0; i < 100; ++i) {
    // Force upcasting to max type.
    uint64_t n1 = GetUniqueHash<TypeParam, TypeParam, TypeParam>(20);
    uint64_t n2 = GetUniqueHash<TypeParam, TypeParam, TypeParam>(20);
    num_collisions += (n1 == n2);
    EXPECT_GE(std::numeric_limits<typename make_unsigned<TypeParam>::type>::max() , n1);
    EXPECT_GE(std::numeric_limits<typename make_unsigned<TypeParam>::type>::max() , n2);
  }
  EXPECT_GE(this->MaxAllowedCollisions(), num_collisions);
}

TYPED_TEST(RandomTest, GetUniqueString) {
  int num_collisions = 0;
  for (int i = 0; i < 100; ++i) {
    // Force upcasting to max type.
    string n1 = GetUniqueString<TypeParam, TypeParam>(20);
    string n2 = GetUniqueString<TypeParam, TypeParam>(20);
    num_collisions += (n1 == n2);
  }
  EXPECT_GE(this->MaxAllowedCollisions(), num_collisions);
}

TYPED_TEST(RandomTest, GetUniqueHashString) {
  int num_collisions = 0;
  for (int i = 0; i < 100; ++i) {
    // Force upcasting to max type.
    string n1 = GetUniqueHashString<TypeParam, TypeParam, TypeParam>(20);
    string n2 = GetUniqueHashString<TypeParam, TypeParam, TypeParam>(20);

    EXPECT_GE(std::numeric_limits<typename make_unsigned<TypeParam>::type>::digits10 + 1,
              n1.size());
    EXPECT_GE(std::numeric_limits<typename make_unsigned<TypeParam>::type>::digits10 + 1,
              n2.size());
    num_collisions += (n1 == n2);
  }
  EXPECT_GE(this->MaxAllowedCollisions(), num_collisions);
}

template<typename T>
class RandomFloatingTest : public testing::Test {
 public:
  virtual ~RandomFloatingTest() {}

 protected:
  int MaxAllowedCollisions() const {
    return 2 / sizeof(T);
  }
};

typedef testing::Types<float, double, long double> FloatingPointTypes;
TYPED_TEST_CASE(RandomFloatingTest, FloatingPointTypes);

TYPED_TEST(RandomFloatingTest, GetNumberInRange) {
  int num_collisions = 0;
  auto generator = GetRandomFloatingPointNumberInRange<TypeParam>(
      static_cast<TypeParam>(0.0), static_cast<TypeParam>(1.0));
  for (int i = 0; i < 100; ++i) {
    TypeParam x = generator();
  }
  EXPECT_GE(this->MaxAllowedCollisions(), num_collisions);
}

TEST(RandomTest, GetRandomValueFromDistribution) {
  size_t seed = 666u;
  float val1 = GetRandomValueFromDistribution(
      std::uniform_real_distribution<float>(0.0, 1.0), seed);
  float val2 = GetRandomValueFromDistribution(
      std::uniform_real_distribution<float>(0.0, 1.0), seed);
  float val3 = GetRandomValueFromDistribution(
      std::uniform_real_distribution<float>(0.0, 1.0));
  EXPECT_FLOAT_EQ(val1, val2);
  EXPECT_NE(val2, val3);
}

}  // namespace test
}  // namespace random
}  // namespace util
