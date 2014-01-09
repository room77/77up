// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/serialization_data.h"

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "test/cc/test_main.h"
#include "util/unique/unique.h"

namespace serial {
namespace test {

namespace {

// Basic data for different tests.
struct Data {
  int a = 0;
  char b_ = 'A';
  string c;
  bool d = true;
  int e[10] = {};
  char f[20] = {};
};

struct Sanity : public Data {
  SERIALIZATION_DATA(a*1 / b_*2 / c*3 / d*4 / e*5 / f*6);
};

struct CustomDefs : public Data {
  SERIALIZATION_DATA(a*1 / DEFAULT_CUSTOM / b_*2 / c*3 / d*4 / DEFAULT_ZERO /
                     e*5 / f*6);
};

struct ForceSerialize : public Data {
  SERIALIZATION_DATA(a*1 / SERIALIZE_ALWAYS / b_*2 / c*3 / d*4 /
                     SERIALIZE_MODIFIED / e*5 / f*6);
};

struct SerializeRequired : public Data {
  SERIALIZATION_DATA(a*1 / SERIALIZE_REQUIRED / b_*2 / c*3 / d*4 /
                     SERIALIZE_OPTIONAL / e*5 / f*6);
};

struct SpecificIds : public Data {
  // The numbers are specified out of order.
  SERIALIZATION_DATA(a*1 / b_*12 / c*2 / d*22 / e*3 / f*4);
};

struct PrettyTypeName : public Data {
  SERIALIZATION_DATA(SERIALIZE_TYPEINFO / a*1);
};

struct NoPrettyTypeName : public Data {
  SERIALIZATION_DATA(a*1);
};

template<typename T>
struct TestData {
  T data;
  SERIALIZATION_DATA(data*1);
};

}  // namespace

TEST(SerializerTest, Sanity) {
  Sanity d;
  EXPECT_EQ(offsetof(Data, a), d.GetSerializationData().field_data("a")->offset);
  EXPECT_EQ(offsetof(Data, b_), d.GetSerializationData().field_data("b")->offset);
  EXPECT_EQ(offsetof(Data, c), d.GetSerializationData().field_data("c")->offset);
  EXPECT_EQ(offsetof(Data, d), d.GetSerializationData().field_data("d")->offset);
  EXPECT_EQ(offsetof(Data, e), d.GetSerializationData().field_data("e")->offset);
  EXPECT_EQ(offsetof(Data, f), d.GetSerializationData().field_data("f")->offset);
}

TEST(SerializerTest, TypeHash) {
  Sanity d;
  EXPECT_EQ(typeid(d.a).hash_code(), d.GetSerializationData().field_data("a")->type_hash);
  EXPECT_EQ(typeid(d.b_).hash_code(), d.GetSerializationData().field_data("b")->type_hash);
  EXPECT_EQ(typeid(d.c).hash_code(), d.GetSerializationData().field_data("c")->type_hash);
  EXPECT_EQ(typeid(d.d).hash_code(), d.GetSerializationData().field_data("d")->type_hash);
  EXPECT_EQ(typeid(d.e).hash_code(), d.GetSerializationData().field_data("e")->type_hash);
  EXPECT_EQ(typeid(d.f).hash_code(), d.GetSerializationData().field_data("f")->type_hash);
}

TEST(SerializerTest, SpecificIds) {
  SpecificIds d;
  EXPECT_EQ(1, d.GetSerializationData().field_data("a")->id);
  EXPECT_EQ(12, d.GetSerializationData().field_data("b")->id);
  EXPECT_EQ(2, d.GetSerializationData().field_data("c")->id);
  EXPECT_EQ(22, d.GetSerializationData().field_data("d")->id);
  EXPECT_EQ(3, d.GetSerializationData().field_data("e")->id);
  EXPECT_EQ(4, d.GetSerializationData().field_data("f")->id);
}

TEST(SerializerTest, CustomDefs) {
  CustomDefs d;
  EXPECT_TRUE(d.GetSerializationData().field_data("a")->zero_default);
  EXPECT_FALSE(d.GetSerializationData().field_data("b")->zero_default);
  EXPECT_FALSE(d.GetSerializationData().field_data("c")->zero_default);
  EXPECT_FALSE(d.GetSerializationData().field_data("d")->zero_default);
  EXPECT_TRUE(d.GetSerializationData().field_data("e")->zero_default);
  EXPECT_TRUE(d.GetSerializationData().field_data("f")->zero_default);
}

TEST(SerializerTest, ForceSerialize) {
  ForceSerialize d;
  EXPECT_FALSE(d.GetSerializationData().field_data("a")->always_serialize);
  EXPECT_TRUE(d.GetSerializationData().field_data("b")->always_serialize);
  EXPECT_TRUE(d.GetSerializationData().field_data("c")->always_serialize);
  EXPECT_TRUE(d.GetSerializationData().field_data("d")->always_serialize);
  EXPECT_FALSE(d.GetSerializationData().field_data("e")->always_serialize);
  EXPECT_FALSE(d.GetSerializationData().field_data("f")->always_serialize);
}

TEST(SerializerTest, SerializeRequired) {
  SerializeRequired d;
  EXPECT_FALSE(d.GetSerializationData().field_data("a")->required);
  EXPECT_TRUE(d.GetSerializationData().field_data("b")->required);
  EXPECT_TRUE(d.GetSerializationData().field_data("c")->required);
  EXPECT_TRUE(d.GetSerializationData().field_data("d")->required);
  EXPECT_FALSE(d.GetSerializationData().field_data("e")->required);
  EXPECT_FALSE(d.GetSerializationData().field_data("f")->required);
}

TEST(SerializerTest, PrettyTypeName) {
  PrettyTypeName d;
  EXPECT_EQ("N6serial4test12_GLOBAL__N_114PrettyTypeNameE",
            d.GetSerializationData().type_info_name());
  EXPECT_EQ("serial::test::(anonymous namespace)::PrettyTypeName",
            d.GetSerializationData().pretty_type_name());

  NoPrettyTypeName e;
  EXPECT_EQ("N6serial4test12_GLOBAL__N_116NoPrettyTypeNameE",
            e.GetSerializationData().type_info_name());
  EXPECT_EQ("", e.GetSerializationData().pretty_type_name());
}

template<typename T>
class SerializerTestType : public ::testing::Test {};

typedef testing::Types<int, char, bool, double, float, long, string,
    pair<int, string>,
    vector<int>,
    map<string, string>, unordered_map<string, string>,
    set<string>, unordered_set<string>,
    shared_ptr<int>, unique_ptr<int>> AllTypes;

TYPED_TEST_CASE(SerializerTestType, AllTypes);

TYPED_TEST(SerializerTestType, Sanity) {
  TestData<TypeParam> d;
  // d.DebugSerializationData();

  EXPECT_EQ(offsetof(TestData<TypeParam>, data),
            d.GetSerializationData().field_data("data")->offset);
  EXPECT_EQ(1, d.GetSerializationData().field_data("data")->id);
}

}  // namespace test
}  // namespace serial
