// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test for binary serialization and deserialization.
// Note: In the future, if the sizes of the basic types change, some of the
// tests will fail. This is preferred to generalization for better sanity
// testing and to avoid code duplication in the test and serialization.

#include "util/serial/type_handlers/binary_serialization.h"
#include "util/serial/type_handlers/binary_deserialization.h"

#include <sstream>

#include "test/cc/unit_test.h"
#include "util/serial/type_handlers/test_util.h"

namespace serial {
namespace test {

TEST(BinarySerializationByTypeTest, TestArithmetic) {
  { int a = -12;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x17", ss.str());

    int b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { int a = 12;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x18", ss.str());

    int b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unsigned int a = 22;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x16", ss.str());

    unsigned int b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { char a = 'A';
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x82\x1", ss.str());

    char b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { bool a = true;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x1", ss.str());

    bool b = false;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { long a = -223;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\xBD\x3", ss.str());

    long b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unsigned long a = 223;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\xDF\x1", ss.str());

    unsigned long b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { float a = 22.3f;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x66\x66\xB2\x41", ss.str());

    float b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { double a = 22.3;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\xCD\xCC\xCC\xCC\xCC\x4C\x36\x40", ss.str());

    double b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { size_t a = 22;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x16", ss.str());

    size_t b = 0;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestString) {
  { string a = "some_string";
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x0Bsome_string", ss.str());

    string b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { string a = "some string";
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x0Bsome string", ss.str());

    string b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestArray) {
  { int a[2] = {1, 2};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x2\x4", ss.str());

    int b[2];
    BinaryDeSerializationByType()(ss, &b);
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a[i], b[i]);
  }

  { string a[2] = {"aa", "bb"};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x02\x02""aa\x02""bb", ss.str());

    string b[2];
    BinaryDeSerializationByType()(ss, &b);
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a[i], b[i]);
  }
}

TEST(BinarySerializationByTypeTest, TestVector) {
  { vector<int> a = {1, 2};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x2\x4", ss.str());

    vector<int> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { vector<bool> a = {true, false};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ(string("\x2\x1\0", 3), ss.str());

    vector<bool> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { vector<string> a = {"aa", "bb"};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x2""aa\x2""bb", ss.str());

    vector<string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestPair) {
  { pair<int, string> a = {1, "a"};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x1""a", ss.str());

    pair<int, string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { pair<vector<int>, vector<string> > a = {{1, 2}, {"a", "b"}};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x2\x4""\x2\x1""a\x1""b", ss.str());

    pair<vector<int>, vector<string> > b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestSet) {
  { set<int> a = {1, 2};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x2\x4", ss.str());

    set<int> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { set<string> a = {"aa", "bb"};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x2""aa\x2""bb", ss.str());

    set<string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestUnorderedSet) {
  { unordered_set<int> a = {1};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x1\x2", ss.str());

    unordered_set<int> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unordered_set<string> a = {"aa"};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x1\x2""aa", ss.str());

    unordered_set<string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestMap) {
  { map<int, string> a = {{1, "a"}, {2, "b"}};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x2\x1""a\x4\x1""b", ss.str());

    map<int,string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { map<string, string> a = {{"A", "a"}, {"B", "b"}};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x2\x1""A\x1""a\x1""B\x1""b", ss.str());

    map<string,string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestUnorderedMap) {
  { unordered_map<int, string> a = {{1, "a"}};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x1\x2\x1""a", ss.str());

    unordered_map<int,string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unordered_map<string, string> a = {{"A", "a"}};
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x1\x1""A\x1""a", ss.str());

    unordered_map<string,string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unordered_map<int, int> a;
    for (int i = 0; i < 1000; ++i) a.emplace(i, i);

    stringstream ss;
    BinarySerializationByType()(ss, a);

    unordered_map<int,int> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a.size(), b.size());
    EXPECT_GT(b.load_factor(), a.load_factor());  // Load factor of b should be higher.
    EXPECT_LT(b.bucket_count(), a.bucket_count());  // Number of buckets of b should be lower.
  }
}

TEST(BinarySerializationByTypeTest, TestUniquePointer) {
  { unique_ptr<int> a(new int(12));
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x18", ss.str());

    unique_ptr<int> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }

  { unique_ptr<string> a(new string("abcd"));
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x4""abcd", ss.str());

    unique_ptr<string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }
}

TEST(BinarySerializationByTypeTest, TestSharedPointer) {
  { shared_ptr<int> a(new int(12));
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x18", ss.str());

    shared_ptr<int> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }

  { shared_ptr<string> a(new string("abcd"));
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("\x4""abcd", ss.str());

    shared_ptr<string> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }
}

TEST(BinarySerializationByTypeTest, TestDataWithZeroDefaults) {
  { TestDataWithSerialization<TestDataWithZeroDefaults> a;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ(string("0 \0  0", 6), ss.str());
  }

  { TestDataWithSerialization<TestDataWithZeroDefaults> a;
    a.test_int = 1;
    a.test_bool = true;
    a.test_char = 'B';
    a.test_str = "some_string";

    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("1 B some_string 1", ss.str());

    TestDataWithSerialization<TestDataWithZeroDefaults> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestDataWithCustomDefaults) {
  { TestDataWithSerialization<TestDataWithCustomDefaults> a;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("1 A some_string 1", ss.str());

    TestDataWithSerialization<TestDataWithCustomDefaults> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { TestDataWithSerialization<TestDataWithCustomDefaults> a;
    a.test_int = 22;
    a.test_bool = false;
    a.test_char = 'F';
    a.test_str = "some_other_string";

    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("22 F some_other_string 0", ss.str());

    TestDataWithSerialization<TestDataWithCustomDefaults> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(BinarySerializationByTypeTest, TestDataWithSerializationDerived) {
  { TestDataWithSerializationDerived<TestDataWithZeroDefaults> a;
    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ(string("0 \0  0", 6), ss.str());
  }

  { TestDataWithSerializationDerived<TestDataWithZeroDefaults> a;
    a.test_int = 1;
    a.test_bool = true;
    a.test_char = 'B';
    a.test_str = "some_string";

    stringstream ss;
    BinarySerializationByType()(ss, a);
    EXPECT_EQ("1 B some_string 1", ss.str());

    TestDataWithSerializationDerived<TestDataWithZeroDefaults> b;
    BinaryDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

}  // namespace test
}  // namespace serial
