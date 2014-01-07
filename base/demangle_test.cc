// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "base/demangle.h"

#include "test/cc/unit_test.h"

namespace base {
namespace test {

namespace {

struct DummyStruct {
  class DummySubclass {
   public:
    string sub_member;
  };
  int member;
};

}  // namespace

TEST(Demangle, PrettyNameFromTypeInfo) {
  EXPECT_EQ("int", PrettyNameFromTypeInfo("i"));
  EXPECT_EQ("bool", PrettyNameFromTypeInfo("b"));
  EXPECT_EQ("char", PrettyNameFromTypeInfo("c"));
  EXPECT_EQ("std::string", PrettyNameFromTypeInfo("Ss"));
  EXPECT_EQ("base::test::(anonymous namespace)::DummyStruct",
            PrettyNameFromTypeInfo(
                "N4base4test12_GLOBAL__N_111DummyStructE"));
  EXPECT_EQ("base::test::(anonymous namespace)::DummyStruct::DummySubclass",
            PrettyNameFromTypeInfo(
                "N4base4test12_GLOBAL__N_111DummyStruct13DummySubclassE"));
}

TEST(Demangle, PrettyNameFromType) {
  EXPECT_EQ("int", PrettyNameFromType<int>());
  EXPECT_EQ("char", PrettyNameFromType<char>());
  EXPECT_EQ("bool", PrettyNameFromType<bool>());
  EXPECT_EQ("std::string", PrettyNameFromType<string>());

  DummyStruct s;
  EXPECT_EQ("base::test::(anonymous namespace)::DummyStruct",
            PrettyNameFromType(s));
  EXPECT_EQ("int", PrettyNameFromType(s.member));

  DummyStruct::DummySubclass c;
  EXPECT_EQ("base::test::(anonymous namespace)::DummyStruct::DummySubclass",
            PrettyNameFromType(c));
  EXPECT_EQ("std::string", PrettyNameFromType(c.sub_member));
}

}  // namespace test
}  // namespace base
