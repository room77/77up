// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test for binary serialization and deserialization.

#include "util/serial/serializer_macros.h"

#include "test/cc/test_main.h"
#include "util/serial/type_handlers/test_util.h"
#include "util/serial/utils/test_util.h"

namespace serial {
namespace test {

TEST(SerializerBinaryTest, TestArithmetic) {
  { TestData<int> a(-12);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x17\0", 3), str);

    TestData<int> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<int> a(12);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x18\0", 3), str);

    TestData<int> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unsigned int> a(22);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x16\0", 3), str);

    TestData<unsigned int> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<char> a('A');
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x82\x1\0", 4), str);

    TestData<char> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<bool> a(true);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x1\0", 3), str);

    TestData<bool> b(false);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<long> a(-223);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\xBD\x3\0", 4), str);

    TestData<long> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unsigned long> a(223);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\xDF\x1\0", 4), str);

    TestData<unsigned long> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<float> a(22.3f);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x9\x66\x66\xB2\x41\0", 6), str);

    TestData<float> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<double> a(22.3);
    string str = a.ToBinary();
    EXPECT_EQ(string("\xA\xCD\xCC\xCC\xCC\xCC\x4C\x36\x40\0", 10), str);

    TestData<double> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<size_t> a(22);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x16\0", 3), str);

    TestData<size_t> b(0);
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestString) {
  { TestData<string> a("some_string");
    string str = a.ToBinary();
    EXPECT_EQ(string("\xB\xB""some_string\0", 14), str);

    TestData<string> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<string> a("some string");
    string str = a.ToBinary();
    EXPECT_EQ(string("\xB\xB""some string\0", 14), str);

    TestData<string> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);

    // Test Parsing Errors.
    EXPECT_FALSE(b.FromBinary(str.substr(0, str.size() - 1)));
    EXPECT_FALSE(b.FromBinary(str.substr(0, str.size() - 3)));
  }
}

TEST(SerializerBinaryTest, TestArray) {
  { TestData<int[2]> a({1, 2});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x3\0\0\0\x2\x2\x4\0", 9), str);

    TestData<int[2]> b({0, 0});
    EXPECT_TRUE(b.FromBinary(str));
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a.data[i], b.data[i]);
  }

  { TestData<string[2]> a({"aa", "bb"});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x7\0\0\0\x2\x2""aa\x2""bb\0", 13), str);

    TestData<string[2]> b;
    EXPECT_TRUE(b.FromBinary(str));
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a.data[i], b.data[i]);
  }
}

TEST(SerializerBinaryTest, TestVector) {
  { TestData<vector<int>> a({1, 2});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x3\0\0\0\x2\x2\x4\0", 9), str);

    TestData<vector<int>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<vector<string>> a({"aa", "bb"});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x7\0\0\0\x2\x2""aa\x2""bb\0", 13), str);

    TestData<vector<string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestPair) {
  { TestData<pair<int, string>> a({1, "a"});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x3\0\0\0\x2\x1""a\0", 9), str);

    TestData<pair<int, string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<pair<vector<int>, vector<string>>> a({{1, 2}, {"a", "b"}});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x8\0\0\0\x2\x2\x4\x2\x1" "a\x1" "b\0", 14), str);

    TestData<pair<vector<int>, vector<string>>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestSet) {
  { TestData<set<int>> a({1, 2});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x3\0\0\0\x2\x2\x4\0", 9), str);

    TestData<set<int>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<set<string>> a({"aa", "bb"});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x7\0\0\0\x2\x2" "aa\x2" "bb\0", 13), str);

    TestData<set<string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestUnorderedSet) {
  { TestData<unordered_set<int>> a({1});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x2\0\0\0\x1\x2\0", 8), str);

    TestData<unordered_set<int>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_set<string>> a({"aa"});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x4\0\0\0\x1\x2" "aa\0", 10), str);

    TestData<unordered_set<string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestMap) {
  { TestData<map<int, string>> a({{1, "a"}, {2, "b"}});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x7\0\0\0\x2\x2\x1" "a\x4\x1" "b\0", 13), str);

    TestData<map<int,string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<map<string, string>> a({{"A", "a"}, {"B", "b"}});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x9\0\0\0\x2\x1" "A\x1" "a\x1" "B\x1" "b\0", 15), str);

    TestData<map<string,string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestUnorderedMap) {
  { TestData<unordered_map<int, string>> a(unordered_map<int, string>{{1, "a"}});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x4\0\0\0\x1\x2\x1" "a\0", 10), str);

    TestData<unordered_map<int,string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_map<string, string>> a(unordered_map<string, string>{{"A", "a"}});
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x5\0\0\0\x1\x1" "A\x1" "a\0", 11), str);

    TestData<unordered_map<string,string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestUniquePointer) {
  {
    TestData<unique_ptr<int>> a(12);
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x18\0", 3), str);

    TestData<unique_ptr<int>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unique_ptr<string>> a("abcd");
    string str = a.ToBinary();
    EXPECT_EQ(string("\xB\x4" "abcd\0", 7), str);

    TestData<unique_ptr<string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestSharedPointer) {
  { TestData<shared_ptr<int>> a(shared_ptr<int>(new int(12)));
    string str = a.ToBinary();
    EXPECT_EQ(string("\x8\x18\0", 3), str);

    TestData<shared_ptr<int>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(*a.data, *b.data);
  }

  { TestData<shared_ptr<string>> a(shared_ptr<string>(new string("abcd")));
    string str = a.ToBinary();
    EXPECT_EQ(string("\xB\x4" "abcd\0", 7), str);

    TestData<shared_ptr<string>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(*a.data, *b.data);
  }
}

TEST(SerializerBinaryTest, TestDataWithZeroDefaults) {
  { TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> a;
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x6\0\0\0""0 \0  0\0", 12), str);
  }

  { TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> a;
    a.data.test_int = 1;
    a.data.test_bool = true;
    a.data.test_char = 'B';
    a.data.test_str = "some_string";

    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x11\0\0\0""1 B some_string 1\0", 23), str);

    TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestDataWithCustomDefaults) {
  { TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> a;
    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x11\0\0\0""1 A some_string 1\0", 23), str);

    TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> a;
    a.data.test_int = 22;
    a.data.test_bool = false;
    a.data.test_char = 'F';
    a.data.test_str = "some_other_string";

    string str = a.ToBinary();
    EXPECT_EQ(string("\xC\x18\0\0\0""22 F some_other_string 0\0", 30), str);

    TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerBinaryTest, TestDataWithAddedRemovedFields) {
  TestDataBasic a(true, 'A', 1, "some_string");
  string str = a.ToBinary();
  EXPECT_EQ(string("\x8\x1\x10\x82\x1\x18\x2\x23\xB""some_string\0", 21), str);
  TestDataBasic b;
  EXPECT_TRUE(b.FromBinary(str));
  EXPECT_EQ(a, b);

  TestDataFieldRemoved c;
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromBinary(str));
  EXPECT_EQ(c, a);

  TestDataFieldAdded d;
  EXPECT_EQ(-1, d.test_added);
  EXPECT_FALSE(d == b);
  EXPECT_TRUE(d.FromBinary(str));
  EXPECT_EQ(d, a);
  EXPECT_EQ(0, d.test_added);

  TestDataFieldAddedRemoved e;
  EXPECT_EQ(-1, e.test_added);
  EXPECT_FALSE(e == a);
  EXPECT_TRUE(e.FromBinary(str));
  EXPECT_EQ(e, a);
  EXPECT_EQ(0, e.test_added);
}

TEST(SerializerBinaryTest, TestNestedData) {
  TestData<TestNested<TestDataBasic> > a;
  a.data.test_basic = { TestDataBasic(true, 'A', 1, "some_string"),
                        TestDataBasic(false, 'B', 23, "some_other_string")};
  string str = a.ToBinary();
  EXPECT_EQ(string(
      "\xC\x37\0\0\0"  // data
        "\xC\x31\0\0\0\x2"  // test_basic
          "\x8\x1\x10\x82\x1\x18\x2\x23\xB""some_string\0"
          "\x8\0\x10\x84\x1\x18\x2E\x23\x11""some_other_string\0"
        "\0"
      "\0", 61), str);

  TestData<TestNested<TestDataBasic>> b;
  EXPECT_TRUE(b.FromBinary(str));
  EXPECT_EQ(a, b);

  TestData<TestNested<TestDataFieldRemoved>> c;
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromBinary(str));
  EXPECT_EQ(c, a);

  TestData<TestNested<TestDataFieldAdded>> d;
  EXPECT_FALSE(d == b);
  EXPECT_TRUE(d.FromBinary(str));
  EXPECT_EQ(d, a);

  TestData<TestNested<TestDataFieldAddedRemoved>> e;
  EXPECT_FALSE(e == a);
  EXPECT_TRUE(e.FromBinary(str));
  EXPECT_EQ(e, a);
}

TEST(SerializerBinaryTest, TestDataWithCallbacks) {
  TestDataBasic a(true, 'A', 1, "some_string");
  string str = a.ToBinary();
  EXPECT_EQ(string("\x8\x1\x10\x82\x1\x18\x2\x23\xB""some_string\0", 21), str);
  TestDataBasic b;
  EXPECT_TRUE(b.FromBinary(str));
  EXPECT_EQ(a, b);

  TestDataWithCallback c;
  EXPECT_TRUE(c.return_value_on_callback);
  EXPECT_FALSE(c.callback);
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromBinary(str));
  EXPECT_EQ(c, a);
  EXPECT_TRUE(c.callback);

  TestDataWithCallback d;
  d.return_value_on_callback = false;
  EXPECT_FALSE(d.callback);
  EXPECT_FALSE(d.FromBinary(str));
  EXPECT_TRUE(d.callback);
}

TEST(SerializerBinaryTest, TestDataRequiredFields) {
  TestDataRequiredFields a(true, 'A', 1, "some_string");
  string str = a.ToBinary();
  EXPECT_EQ(string("\x8\x1\x10\x82\x1\x18\x2\x23\xB""some_string\0", 21), str);
  {
    TestDataBasic b;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);

    TestDataRequiredFields c;
    EXPECT_TRUE(b.FromBinary(str));
    EXPECT_EQ(a, b);
  }
  {
    // Remove the test_char field.
    string str("\x8\x1\x18\x2\x23\xB""some_string\0", 18);
    TestDataBasic b;
    EXPECT_TRUE(b.FromBinary(str));

    TestDataRequiredFields c;
    EXPECT_FALSE(c.FromBinary(str));
  }
  {
    // Remove the test_int field.
    string str("\x8\x1\x10\x82\x1\x23\xB""some_string\0", 19);
    TestDataBasic b;
    EXPECT_TRUE(b.FromBinary(str));

    TestDataRequiredFields c;
    EXPECT_FALSE(c.FromBinary(str));
  }
}

TEST(SerializerBinaryTest, TestVirtual) {
  TestDataVirtualDerived a;
  TestDataVirtualDerived b;
  a.test_int = 1;
  a.test_float = 2.0;
  const TestDataVirtualBase& c = a;
  b.FromBinary(c.ToBinary());
  EXPECT_FLOAT_EQ(b.test_float, a.test_float);
}

}  // namespace test
}  // namespace serial
