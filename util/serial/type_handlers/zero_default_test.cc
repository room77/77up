// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/type_handlers/zero_default.h"

#include "test/cc/unit_test.h"
#include "util/serial/type_handlers/test_util.h"

namespace serial {
namespace test {

TEST(ZeroDefaultTest, TestArithmetic) {
  { int a = -12;
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }

  { unsigned int a = 22;
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }

  { char a = 'A';
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }

  { bool a = true;
    DefaultZeroByType()(a);
    EXPECT_FALSE(a);
  }

  { long a = -223;
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }

  { unsigned long a = 223;
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }

  { float a = 22.3;
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }

  { double a = 22.3;
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }

  { size_t a = 22.3;
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a);
  }
}

TEST(ZeroDefaultTest, TestString) {
  string a = "some_string";
  DefaultZeroByType()(a);
  EXPECT_TRUE(a.empty());
}

TEST(ZeroDefaultTest, TestArray) {
  { int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    DefaultZeroByType()(a);
    int def[10] = {};
    EXPECT_EQ(0, memcmp(def, a, 10));
  }

  { string a[10] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    DefaultZeroByType()(a);
    string def[10] = {};
    EXPECT_EQ(0, memcmp(def, a, 10));
  }
}

TEST(ZeroDefaultTest, TestVector) {
  { vector<int> a = {1, 2, 3, 4};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }

  { vector<string> a = {"1", "2", "3", "4"};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }
}

TEST(ZeroDefaultTest, TestPair) {
  { pair<int, string> a = {1, "1"};
    DefaultZeroByType()(a);
    EXPECT_EQ(0, a.first);
    EXPECT_EQ("", a.second);
  }

  { pair<vector<int>, vector<string> > a = {{1, 2, 3, 4}, {"1", "2", "3", "4"}};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.first.empty());
    EXPECT_TRUE(a.second.empty());
  }
}

TEST(ZeroDefaultTest, TestSet) {
  { set<int> a = {1, 2, 3, 4};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }

  { set<string> a = {"1", "2", "3", "4"};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }
}

TEST(ZeroDefaultTest, TestUnorderedSet) {
  { unordered_set<int> a = {1, 2, 3, 4};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }

  { unordered_set<string> a = {"1", "2", "3", "4"};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }
}

TEST(ZeroDefaultTest, TestMap) {
  { map<int, string> a = {{1, "1"}, {2, "2"}, {3, "3"}, {3, "3"}};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }

  { map<string, string> a = {{"1", "1"}, {"2", "2"}, {"3", "3"}, {"3", "3"}};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }
}

TEST(ZeroDefaultTest, TestUnorderedMap) {
  { unordered_map<int, string> a = {{1, "1"}, {2, "2"}, {3, "3"}, {3, "3"}};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }

  { unordered_map<string, string> a = {{"1", "1"}, {"2", "2"}, {"3", "3"},
                                        {"3", "3"}};
    DefaultZeroByType()(a);
    EXPECT_TRUE(a.empty());
  }
}

TEST(ZeroDefaultTest, TestUniquePointer) {
  { unique_ptr<int> a(new int(12));
    EXPECT_EQ(12, *a);
    DefaultZeroByType()(a);
    EXPECT_EQ(0, *a);
  }

  { unique_ptr<string> a(new string("1234"));
    EXPECT_EQ("1234", *a);
    DefaultZeroByType()(a);
    EXPECT_EQ("", *a);
  }
}

TEST(ZeroDefaultTest, TestSharedPointer) {
  { shared_ptr<int> a(new int(12));
    EXPECT_EQ(12, *a);
    DefaultZeroByType()(a);
    EXPECT_EQ(0, *a);
  }

  { shared_ptr<string> a(new string("1234"));
    EXPECT_EQ("1234", *a);
    DefaultZeroByType()(a);
    EXPECT_EQ("", *a);
  }
}

TEST(ZeroDefaultTest, TestDataWithZeroDefaults) {
  TestDataWithZeroDefaults a, def_val;
  EXPECT_EQ(def_val, a);

  a.test_int = 1;
  a.test_set = {1 , 2, 3};
  a.test_map = {{1, "S1"} ,{2, "S2"}};
  EXPECT_FALSE(def_val == a);

  DefaultZeroByType()(a);
  EXPECT_EQ(def_val, a);
}

TEST(ZeroDefaultTest, TestDataWithCustomDefaults) {
  TestDataWithCustomDefaults a, def_val;
  EXPECT_EQ(def_val, a);

  a.test_int += 1;
  a.test_set = {1 , 2, 3};
  a.test_map = {{1, "S1"} ,{2, "S2"}};
  EXPECT_FALSE(def_val == a);

  DefaultZeroByType()(a);
  EXPECT_EQ(def_val, a);
}

}  // namespace test
}  // namespace serial
