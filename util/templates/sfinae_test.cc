// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/templates/sfinae.h"

#include <map>
#include <memory>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "test/cc/test_main.h"

namespace sfinae {
namespace test {

namespace {

struct FooBase {
  void func(int) {}
  void func(double) const {}

  typedef int TypeInFooBase;
};

struct FooS {
   void func(string) {}
};

struct DerFooD : public FooBase {};

struct DerFooS : public FooS {};

struct DerFooDS : public FooBase, public FooS {
  using FooBase::func;
  using FooS::func;

  typedef int TypeInDerFooDS;
};

}  // namespace

// ****************************************************************
// Tests for CREATE_MEMBER_FUNC_SOFT_SIG_CHECK
// ****************************************************************
CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(func);
TEST(SfinaeSoftFunctionSignature, Sanity) {
  // Verify FooBase.
  static_assert(has_member_func_sig_func<const FooBase, void(double)>::value,
                "Failed FooBase double");
  static_assert(!has_member_func_sig_func<FooBase, void()>::value,
                "Failed FooBase void");
  static_assert(!has_member_func_sig_func<FooBase, void(string)>::value,
                "Failed FooBase string");

  // Verify FooS.
  static_assert(!has_member_func_sig_func<const FooS, void(double)>::value,
                "Failed FooS double");
  static_assert(!has_member_func_sig_func<FooS, void()>::value,
                "Failed FooS void");
  static_assert(has_member_func_sig_func<FooS, void(string)>::value,
                "Failed FooS string");

  // Verify DerFooD.
  static_assert(has_member_func_sig_func<const DerFooD, void(double)>::value,
                "Failed DerFooD double");
  static_assert(!has_member_func_sig_func<DerFooD, void()>::value,
                "Failed DerFooD void");
  static_assert(!has_member_func_sig_func<DerFooD, void(string)>::value,
                "Failed DerFooD string");

  // Verify DerFooS.
  static_assert(!has_member_func_sig_func<const DerFooS, void(double)>::value,
                "Failed DerFooS double");
  static_assert(!has_member_func_sig_func<DerFooS, void()>::value,
                "Failed DerFooS void");
  static_assert(has_member_func_sig_func<DerFooS, void(string)>::value,
                "Failed DerFooS string");

  // Verify DerFooDS.
  static_assert(has_member_func_sig_func<const DerFooDS, void(double)>::value,
                "Failed DerFooDS double");
  static_assert(!has_member_func_sig_func<DerFooDS, void()>::value,
                "Failed DerFooDS void");
  static_assert(has_member_func_sig_func<DerFooDS, void(string)>::value,
                "Failed DerFooDS string");
}

// Test Method 'create'.

CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(clear);

TEST(SfinaeSoftFunctionSignature, NoClear) {
  static_assert(!has_member_func_sig_clear<FooBase, void()>::value,
                "Failed FooBase");

  static_assert(!has_member_func_sig_clear<int, void()>::value,
                "Failed int");
}

template<typename T>
class SfinaeSoftFunctionSignatureClearTest : public ::testing::Test {};
typedef testing::Types<vector<int>, set<char>, unordered_set<string>,
    map<int, double>, unordered_map<string, float> > AllClearTypes;
TYPED_TEST_CASE(SfinaeSoftFunctionSignatureClearTest, AllClearTypes);

TYPED_TEST(SfinaeSoftFunctionSignatureClearTest, Sanity) {
  static_assert(has_member_func_sig_clear<TypeParam, void()>::value, "");
}

// Test Method 'reset'.

CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(reset);

TEST(SfinaeSoftFunctionSignature, NoReset) {
  static_assert(!has_member_func_sig_reset<FooBase, void()>::value,
                "Failed FooBase");

  static_assert(!has_member_func_sig_reset<int, void()>::value,
                "Failed int");
}

template<typename T>
class SfinaeSoftFunctionSignatureResetTest : public ::testing::Test {};
typedef testing::Types<unique_ptr<int>, shared_ptr<string>> AllResetTypes;
TYPED_TEST_CASE(SfinaeSoftFunctionSignatureResetTest, AllResetTypes);

TYPED_TEST(SfinaeSoftFunctionSignatureResetTest, Sanity) {
  static_assert(has_member_func_sig_reset<TypeParam, void()>::value, "");
}

// Test Method 'insert'.

CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(insert);

template<typename T>
class SfinaeSoftFunctionSignatureInsertTest : public ::testing::Test {};
typedef testing::Types<string, vector<int>, unordered_map<string, int> > AllInsertTypes;
TYPED_TEST_CASE(SfinaeSoftFunctionSignatureInsertTest, AllInsertTypes);

TYPED_TEST(SfinaeSoftFunctionSignatureInsertTest, Sanity) {
  string s;
  static_assert(has_member_func_sig_insert<TypeParam, typename TypeParam::iterator(typename TypeParam::iterator, const typename TypeParam::value_type&)>::value, "");
}

// ****************************************************************
// Tests for CREATE_MEMBER_FUNC_EXACT_SIG_CHECK
// ****************************************************************
CREATE_MEMBER_FUNC_EXACT_SIG_CHECK(func);

TEST(SfinaeExactFunctionSignature, Sanity) {
  // Verify FooBase.
  static_assert(has_member_func_exact_sig_func<FooBase, void(int)>::value,
                "Failed FooBase int");

  static_assert(has_member_func_exact_sig_func<FooBase, void (double) const>::value,
                "Failed FooBase double");
  static_assert(!has_member_func_exact_sig_func<FooBase, void()>::value,
                "Failed FooBase void");
  static_assert(!has_member_func_exact_sig_func<FooBase, void(string)>::value,
                "Failed FooBase string");

  // Verify FooS.
  static_assert(!has_member_func_exact_sig_func<FooS, void(double) const>::value,
                "Failed FooS double");
  static_assert(!has_member_func_exact_sig_func<FooS, void()>::value,
                "Failed FooS void");
  static_assert(has_member_func_exact_sig_func<FooS, void(string)>::value,
                "Failed FooS string");

  // Verify DerFooD.
  static_assert(!has_member_func_exact_sig_func<DerFooD, void(double) const>::value,
                "Failed DerFooD double");
  static_assert(!has_member_func_exact_sig_func<DerFooD, void()>::value,
                "Failed DerFooD void");
  static_assert(!has_member_func_exact_sig_func<DerFooD, void(string)>::value,
                "Failed DerFooD string");

  // Verify DerFooS.
  static_assert(!has_member_func_exact_sig_func<DerFooS, void(double) const>::value,
                "Failed DerFooS double");
  static_assert(!has_member_func_exact_sig_func<DerFooS, void()>::value,
                "Failed DerFooS void");
  static_assert(!has_member_func_exact_sig_func<DerFooS, void(string)>::value,
                "Failed DerFooS string");

  // Verify DerFooDS.
  static_assert(!has_member_func_exact_sig_func<DerFooDS, void(double) const>::value,
                "Failed DerFooDS double");
  static_assert(!has_member_func_exact_sig_func<DerFooDS, void()>::value,
                "Failed DerFooDS void");
  static_assert(!has_member_func_exact_sig_func<DerFooDS, void(string)>::value,
                "Failed DerFooDS string");
}

// ****************************************************************
// Tests for CREATE_MEMBER_TYPE_CHECK
// ****************************************************************

CREATE_MEMBER_TYPE_CHECK(TypeInFooBase);

TEST(SfinaeType, TypeInFooBase) {
  static_assert(has_member_type_TypeInFooBase<FooBase>::value, "Failed FooBase");
  static_assert(!has_member_type_TypeInFooBase<FooS>::value, "Failed FooS");

  static_assert(has_member_type_TypeInFooBase<DerFooD>::value, "Failed DerFooD");
  static_assert(has_member_type_TypeInFooBase<DerFooDS>::value, "Failed DerFooDS");
}

CREATE_MEMBER_TYPE_CHECK(const_iterator);

TEST(SfinaeType, NoConstIterator) {
  static_assert(!has_member_type_const_iterator<FooBase>::value, "Failed FooBase");

  static_assert(!has_member_type_const_iterator<int>::value, "Failed int");
}

template<typename T>
class SfinaeTypeConstIteratorTest : public ::testing::Test {};
typedef testing::Types<map<int, int>, set<int>, vector<int>,
    unordered_map<int, int>, unordered_set<int>> AllConstIteratorTypes;
TYPED_TEST_CASE(SfinaeTypeConstIteratorTest, AllConstIteratorTypes);

TYPED_TEST(SfinaeTypeConstIteratorTest, Sanity) {
  static_assert(has_member_type_const_iterator<TypeParam>::value, "");
}

}  // namespace test
}  // namespace sfinae
