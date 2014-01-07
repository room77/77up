// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/type_handlers/type_size.h"

#include "util/serial/type_handlers/test_util.h"
#include "test/cc/unit_test.h"

namespace serial {
namespace test {

template<typename T>
class SizeByTypeTestSizeIntegerTypes : public ::testing::Test {};
TYPED_TEST_CASE(SizeByTypeTestSizeIntegerTypes, AllIntegerTypes);
TYPED_TEST(SizeByTypeTestSizeIntegerTypes, Sanity) {
  EXPECT_EQ(kSerialTypeSizeVarInt, SizeByType<TypeParam>::value);
}

template<typename T>
class SizeByTypeTestSizeFloating : public ::testing::Test {};
TYPED_TEST_CASE(SizeByTypeTestSizeFloating, AllFloatingTypes);
TYPED_TEST(SizeByTypeTestSizeFloating, Sanity) {
  int size = sizeof(TypeParam) == 4 ? kSerialTypeSizeFour : kSerialTypeSizeEight;
  EXPECT_EQ(size, SizeByType<TypeParam>::value);
}

template<typename T>
class SizeByTypeTestSizeVariableTypes : public ::testing::Test {};
TYPED_TEST_CASE(SizeByTypeTestSizeVariableTypes, AllVarTypes);
TYPED_TEST(SizeByTypeTestSizeVariableTypes, Sanity) {
  EXPECT_EQ(kSerialTypeSizeVarInt, SizeByType<TypeParam>::value);
}

template<typename T>
class SizeByTypeTestSizeFixedTypes : public ::testing::Test {};
TYPED_TEST_CASE(SizeByTypeTestSizeFixedTypes, AllFixedTypes);
TYPED_TEST(SizeByTypeTestSizeFixedTypes, Sanity) {
  int size = sizeof(typename TypeParam::type) == 4 ? kSerialTypeSizeFour : kSerialTypeSizeEight;
  EXPECT_EQ(size, SizeByType<TypeParam>::value);
}

template<typename T>
class SizeByTypeTestSizeContainerTypes : public ::testing::Test {};
TYPED_TEST_CASE(SizeByTypeTestSizeContainerTypes, AllContainerTypes);
TYPED_TEST(SizeByTypeTestSizeContainerTypes, Sanity) {
  EXPECT_EQ(kSerialTypeUnknownFixedInt, SizeByType<TypeParam>::value);
}

TEST(SizeByTypeTest, PointerTypes) {
  EXPECT_EQ(kSerialTypeSizeVarInt, SizeByType<int*>::value);
  EXPECT_EQ(kSerialTypeUnknownVarInt, SizeByType<string*>::value);
  EXPECT_EQ(kSerialTypeUnknownFixedInt, SizeByType<TestDataWithZeroDefaults*>::value);

  EXPECT_EQ(kSerialTypeSizeVarInt, SizeByType<shared_ptr<int> >::value);
  EXPECT_EQ(kSerialTypeUnknownVarInt, SizeByType<shared_ptr<string> >::value);
  EXPECT_EQ(kSerialTypeUnknownFixedInt, SizeByType<shared_ptr<TestDataWithZeroDefaults> >::value);

  EXPECT_EQ(kSerialTypeSizeVarInt, SizeByType<unique_ptr<int> >::value);
  EXPECT_EQ(kSerialTypeUnknownVarInt, SizeByType<unique_ptr<string> >::value);
  EXPECT_EQ(kSerialTypeUnknownFixedInt, SizeByType<unique_ptr<TestDataWithZeroDefaults> >::value);
}


TEST(SizeByTypeTest, Miscellaneous) {
  EXPECT_EQ(kSerialTypeUnknownFixedInt, SizeByType<TestDataWithZeroDefaults>::value);
  EXPECT_EQ(kSerialTypeUnknownFixedInt, SizeByType<TestDataWithCustomDefaults>::value);
  EXPECT_EQ(kSerialTypeUnknownVarInt, SizeByType<string>::value);
}

}  // namespace test
}  // namespace serial
