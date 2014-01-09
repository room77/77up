// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test for raw binary serialization and deserialization.

#include "util/serial/serializer_macros.h"

#include "test/cc/test_main.h"
#include "util/serial/utils/test_util.h"
#include "util/serial/type_handlers/test_util.h"

namespace serial {
namespace test {

TEST(SerializerRawBinaryTest, TestArithmetic) {
  { TestData<int> a(-12);
    string str = a.ToRawBinary();
    EXPECT_EQ("\x17", str);

    TestData<int> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<int> a(12);
    string str = a.ToRawBinary();
    EXPECT_EQ("\x18", str);

    TestData<int> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unsigned int> a(22);
    string str = a.ToRawBinary();
    EXPECT_EQ("\x16", str);

    TestData<unsigned int> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<char> a('A');
    string str = a.ToRawBinary();
    EXPECT_EQ("\x82\x1", str);

    TestData<char> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<bool> a(true);
    string str = a.ToRawBinary();
    EXPECT_EQ("\x1", str);

    TestData<bool> b(false);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<long> a(-223);
    string str = a.ToRawBinary();
    EXPECT_EQ("\xBD\x3", str);

    TestData<long> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unsigned long> a(223);
    string str = a.ToRawBinary();
    EXPECT_EQ("\xDF\x1", str);

    TestData<unsigned long> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<float> a(22.3f);
    string str = a.ToRawBinary();
    EXPECT_EQ("\x66\x66\xB2\x41", str);

    TestData<float> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<double> a(22.3);
    string str = a.ToRawBinary();
    EXPECT_EQ("\xCD\xCC\xCC\xCC\xCC\x4C\x36\x40", str);

    TestData<double> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<size_t> a(22);
    string str = a.ToRawBinary();
    EXPECT_EQ("\x16", str);

    TestData<size_t> b(0);
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestString) {
  { TestData<string> a("some_string");
    string str = a.ToRawBinary();
    EXPECT_EQ("\xb""some_string", str);

    TestData<string> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<string> a("some string");
    string str = a.ToRawBinary();
    EXPECT_EQ("\xb""some string", str);

    TestData<string> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);

    // Test Parsing Errors.
    EXPECT_FALSE(b.FromRawBinary(str.substr(0, str.size() - 3)));
  }
}

TEST(SerializerRawBinaryTest, TestArray) {
  { TestData<int[2]> a({1, 2});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2\x4", str);

    TestData<int[2]> b({0, 0});
    EXPECT_TRUE(b.FromRawBinary(str));
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a.data[i], b.data[i]);
  }

  { TestData<string[2]> a({"aa", "bb"});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2""aa\x2""bb", str);

    TestData<string[2]> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a.data[i], b.data[i]);
  }
}

TEST(SerializerRawBinaryTest, TestVector) {
  { TestData<vector<int>> a({1, 2});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2\x4", str);

    TestData<vector<int>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<vector<string>> a({"aa", "bb"});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2""aa\x2""bb", str);

    TestData<vector<string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestPair) {
  { TestData<pair<int, string>> a({1, "a"});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x1""a", str);

    TestData<pair<int, string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<pair<vector<int>, vector<string>>> a({{1, 2}, {"a", "b"}});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2\x4\x2\x1""a\x1""b", str);

    TestData<pair<vector<int>, vector<string>>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestSet) {
  { TestData<set<int>> a({1, 2});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2\x4", str);

    TestData<set<int>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<set<string>> a({"aa", "bb"});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2""aa\x2""bb", str);

    TestData<set<string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestUnorderedSet) {
  { TestData<unordered_set<int>> a({1});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x1\x2", str);

    TestData<unordered_set<int>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_set<string>> a({"aa"});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x1\x2""aa", str);

    TestData<unordered_set<string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestMap) {
  { TestData<map<int, string>> a({{1, "a"}, {2, "b"}});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x2\x1""a\x4\x1""b", str);

    TestData<map<int, string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<map<string, string>> a({{"A", "a"}, {"B", "b"}});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x2\x1""A\x1""a\x1""B\x1""b", str);

    TestData<map<string, string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestUnorderedMap) {
  { TestData<unordered_map<int, string>> a(unordered_map<int, string>{{1, "a"}});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x1\x2\x1""a", str);

    TestData<unordered_map<int, string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_map<string, string>> a(unordered_map<string, string>{{"A", "a"}});
    string str = a.ToRawBinary();
    EXPECT_EQ("\x1\x1""A\x1""a", str);

    TestData<unordered_map<string, string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestUniquePointer) {
  {
    TestData<unique_ptr<int>> a(12);
    string str = a.ToRawBinary();
    EXPECT_EQ("\x18", str);

    TestData<unique_ptr<int>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unique_ptr<string>> a("abcd");
    string str = a.ToRawBinary();
    EXPECT_EQ("\x4""abcd", str);

    TestData<unique_ptr<string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestSharedPointer) {
  { TestData<shared_ptr<int>> a(shared_ptr<int>(new int(12)));
    string str = a.ToRawBinary();
    EXPECT_EQ("\x18", str);

    TestData<shared_ptr<int>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(*a.data, *b.data);
  }

  { TestData<shared_ptr<string>> a(shared_ptr<string>(new string("abcd")));
    string str = a.ToRawBinary();
    EXPECT_EQ("\x4""abcd", str);

    TestData<shared_ptr<string>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(*a.data, *b.data);
  }
}

TEST(SerializerRawBinaryTest, TestDataWithZeroDefaults) {
  { TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> a;
    string str = a.ToRawBinary();
    EXPECT_EQ(string("0 \0  0", 6), str);
  }

  { TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> a;
    a.data.test_int = 1;
    a.data.test_bool = true;
    a.data.test_char = 'B';
    a.data.test_str = "some_string";

    string str = a.ToRawBinary();
    EXPECT_EQ("1 B some_string 1", str);

    TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestDataWithCustomDefaults) {
  { TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> a;
    string str = a.ToRawBinary();
    EXPECT_EQ("1 A some_string 1", str);

    TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }

  { TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> a;
    a.data.test_int = 22;
    a.data.test_bool = false;
    a.data.test_char = 'F';
    a.data.test_str = "some_other_string";

    string str = a.ToRawBinary();
    EXPECT_EQ("22 F some_other_string 0", str);

    TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> b;
    EXPECT_TRUE(b.FromRawBinary(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerRawBinaryTest, TestDataWithBasicFields) {
  TestDataBasic a(true, 'A', 1, "some_string");
  string str = a.ToRawBinary();
  EXPECT_EQ("\x1\x82\x1\x2\xB""some_string", str);
  TestDataBasic b;
  EXPECT_TRUE(b.FromRawBinary(str));
  EXPECT_EQ(a, b);
}

TEST(SerializerRawBinaryTest, TestNestedData) {
  TestData<TestNested<TestDataBasic> > a;
  a.data.test_basic = { TestDataBasic(true, 'A', 1, "some_string"),
                        TestDataBasic(false, 'B', 23, "some_other_string")};
  string str = a.ToRawBinary();
  EXPECT_EQ(string("\x2"
          "\x1\x82\x1\x2\xB""some_string"
          "\0\x84\x1\x2e\x11""some_other_string", 39), str);
  TestData<TestNested<TestDataBasic>> b;
  EXPECT_TRUE(b.FromRawBinary(str));
  EXPECT_EQ(a, b);
}

TEST(SerializerRawBinaryTest, TestDataWithCallbacks) {
  TestDataBasic a(true, 'A', 1, "some_string");
  string str = a.ToRawBinary();
  EXPECT_EQ("\x1\x82\x1\x2\xB""some_string", str);
  TestDataBasic b;
  EXPECT_TRUE(b.FromRawBinary(str));
  EXPECT_EQ(a, b);

  TestDataWithCallback c;
  EXPECT_FALSE(c.callback);
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromRawBinary(str));
  EXPECT_EQ(c, a);
  EXPECT_TRUE(c.callback);

  TestDataWithCallback d;
  d.return_value_on_callback = false;
  EXPECT_FALSE(d.callback);
  EXPECT_FALSE(d.FromRawBinary(str));
  EXPECT_TRUE(d.callback);
}

TEST(SerializerRawBinaryTest, TestVirtual) {
  TestDataVirtualDerived a;
  TestDataVirtualDerived b;
  a.test_int = 1;
  a.test_float = 2.0;
  const TestDataVirtualBase& c = a;
  b.FromRawBinary(c.ToRawBinary());
  EXPECT_FLOAT_EQ(b.test_float, a.test_float);
}

}  // namespace test
}  // namespace serial
