// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/templates/container_util.h"

#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "test/cc/test_main.h"

namespace util {
namespace tl {
namespace test {

template<typename T>
class ContainerUtilTest : public ::testing::Test {};

typedef testing::Types<set<int>, unordered_set<int> > AllSets;
TYPED_TEST_CASE(ContainerUtilTest, AllSets);

TYPED_TEST(ContainerUtilTest, Sanity) {
  TypeParam c;
  int p = 1;
  EXPECT_EQ(nullptr, FindOrNull(c, p));
  ASSERT_DEATH(FindOrDie(c, p), "");
  EXPECT_EQ(1, FindWithDefault(c, 1, p));
  EXPECT_EQ(1, FindWithInsert(c, 1, p));
  EXPECT_NE(nullptr, FindOrNull(c, 1));

  EXPECT_EQ(nullptr, FindOrNull(c, 2));
  ASSERT_DEATH(FindOrDie(c, 2), "");
  EXPECT_EQ(2, FindWithInsert(c, 2));
  EXPECT_NE(nullptr, FindOrNull(c, 2));
}

template<typename T>
class ContainerUtilTestKV : public ::testing::Test {};

typedef testing::Types<map<int, int>, unordered_map<int, int> > AllMaps;
TYPED_TEST_CASE(ContainerUtilTestKV, AllMaps);

TYPED_TEST(ContainerUtilTestKV, Sanity) {
  TypeParam c;
  pair<const int, int> p(1, 1);
  EXPECT_EQ(nullptr, FindOrNull(c, 1));
  ASSERT_DEATH(FindOrDie(c, 1), "");
  ASSERT_DEATH(FindMappedOrDie(c, 1), "");
  EXPECT_EQ(p, FindWithDefault(c, 1, p));
  EXPECT_EQ(p, FindWithInsert(c, 1, p));
  EXPECT_NE(nullptr, FindOrNull(c, 1));
  EXPECT_EQ(p, FindOrDie(c, 1));

  EXPECT_EQ(nullptr, FindOrNull(c, 2));
  ASSERT_DEATH(FindOrDie(c, 2), "");
  ASSERT_DEATH(FindMappedOrDie(c, 2), "");
  EXPECT_EQ(2, FindWithDefault(c, 2, 2));
  EXPECT_EQ(2, FindWithInsert(c, 2, 2));
  EXPECT_NE(nullptr, FindOrNull(c, 2));
  EXPECT_EQ(2, FindMappedOrDie(c, 2));
}

TEST(ContainerUtil, GetOrdering) {
  vector<string> list = {"b", "c", "a", "d"};
  vector<int> expected_ordering = {1, 2, 0, 3};
  vector<int> ordering = GetOrdering(list);
  EXPECT_EQ(ordering, expected_ordering);
}


TEST(ContainerUtil, GetOrdering_with_comp) {
  vector<string> list = {"b", "c", "a", "d"};
  vector<int> nums = {6, 4, 0, 2};
  auto comp = [&nums](int i, int j) { return nums[i] < nums[j]; };
  vector<int> expected_ordering = {3, 2, 0, 1};
  vector<int> ordering = GetOrderingBy(list.size(), comp);
  EXPECT_EQ(ordering, expected_ordering);
}

TEST(ContainerUtil, GetSortIndices) {
  vector<string> list = {"b", "c" , "a", "d"};
  vector<int> expected_indices = {2, 0, 1, 3};
  vector<int> indices = GetSortIndices(list);
  EXPECT_EQ(indices, expected_indices);
}


TEST(ContainerUtil, GetSortIndices_repeated_values) {
  vector<int> list = {1,0,1,1,0,1};
  vector<int> indices = GetSortIndices(list);
  int last_val = 0;
  for (int i : indices) {
    EXPECT_GE(list[i], last_val);
    last_val = list[i];
  }
}

TEST(ContainerUtil, Zip) {
  vector<int> v = { 5, 3, 2 };
  auto double_v = Zip( std::plus<int>(), v, v );
  EXPECT_EQ(double_v.size(), v.size());
  for (int i = 0 ; i < v.size() ; ++i) {
    EXPECT_EQ(double_v[i], v[i] * 2);
  }
}

template<typename T>
class MapTest : public ::testing::Test {};

typedef testing::Types<set<int>, unordered_set<int>, vector<int>, deque<int> > IntContainers;
TYPED_TEST_CASE(MapTest, IntContainers);

TEST(ContainerUtil, Map) {
  vector<int> v = { 5, 3, 2 };
  vector<int> double_v = Map([](int i) { return i * 2; }, v);
  EXPECT_EQ(double_v.size(), v.size());
  for (int i = 0 ; i < v.size() ; ++i) {
    EXPECT_EQ(double_v[i], v[i] * 2);
  }
}

TYPED_TEST(MapTest, Map_with_type_change) {
  vector<int> v = { 5, 3, 2 };
  auto double_v = Map<TypeParam>([](int i) { return i * 2; }, v);
  EXPECT_EQ(double_v.size(), v.size());
}

TEST(ContainerUtil, Map_to_unordered_map) {
  vector<int> v = { 5, 3, 2 };
  unordered_map<int, int> double_v = Map<unordered_map<int, int> >([](int i) { return make_pair(i, i * 2); }, v);
  EXPECT_EQ(double_v.size(), v.size());
  for (int i = 0 ; i < v.size() ; ++i) {
    EXPECT_EQ(double_v[v[i]], v[i] * 2);
  }
}

TEST(GetKeyValueTest, GetKeys) {
  map<int, int> m = {{0, 1}, {2, 3}, {4, 5}};
  vector<int> expected_keys = {0, 2, 4};
  auto keys = GetKeys(m);
  EXPECT_EQ(keys, expected_keys);
}

TEST(GetKeyValueTest, GetValues) {
  map<int, int> m = {{0, 1}, {2, 3}, {4, 5}};
  vector<int> expected_values = {1, 3, 5};
  auto values = GetValues(m);
  EXPECT_EQ(values, expected_values);
}

TEST(ContainerUtil, Map_from_unordered_map) {
  unordered_map<int, int> v = {{5, 2}, {3, 1}, {2, 4}};
  vector<int> mult_v = Map<vector<int> >([](const unordered_map<int, int>::value_type& p) { return p.first * p.second; }, v);
  EXPECT_EQ(mult_v.size(), v.size());
}

TEST(ContainerUtil, Filter) {
  vector<int> naturals = {1,2,3,4,5,6,7,8,9,10};
  auto evens = Filter([](int x) { return x%2==0; }, naturals);
  EXPECT_EQ(evens.size(), naturals.size() / 2);
  for (int i = 0 ; i < evens.size() ; ++i) {
    EXPECT_EQ(evens[i] % 2, 0);
  }
}

template<typename T>
class MaxTest : public ::testing::Test {};

typedef testing::Types<set<int>, unordered_set<int>, vector<int>, deque<int> > IntContainers;
TYPED_TEST_CASE(MaxTest, IntContainers);

TYPED_TEST(MaxTest, TestMax) {
  TypeParam c = {4, 2, 3, 4, 5, 1};
  EXPECT_EQ(5, Max(c));
}

TYPED_TEST(MaxTest, TestMax_with_comparator) {
  TypeParam c = {4, 2, 3, 4, 5, 1};
  auto comp = [](int i, int j) { return abs(i - 4) < abs(j - 4); };
  EXPECT_EQ(1, Max(c, comp));
}


}  // namespace test
}  // namespace tl
}  // namespace util
