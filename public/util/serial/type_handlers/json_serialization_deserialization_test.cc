// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test for JSON serialization and deserialization.

#include "util/serial/type_handlers/json_serialization.h"
#include "util/serial/type_handlers/json_deserialization.h"

#include <sstream>

#include "test/cc/test_main.h"
#include "util/serial/type_handlers/test_util.h"

namespace serial {
namespace test {

TEST(JSONSerializationByTypeTest, TestArithmetic) {
  { int a = 2147483647;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("2147483647", ss.str());

    int b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
  { int a = 12;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("12", ss.str());

    int b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
  { int a = 0;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("0", ss.str());

    int b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { int a = 12;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("12", ss.str());

    int b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unsigned int a = 22;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("22", ss.str());

    unsigned int b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { char a = 'A';
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("65", ss.str());

    char b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { bool a = true;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("true", ss.str());

    bool b = false;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { bool a = false;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("false", ss.str());

    bool b = true;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }


  { long a = -223;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("-223", ss.str());

    long b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unsigned long a = 223;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("223", ss.str());

    unsigned long b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { float a = 22.3f;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("22.3", ss.str());

    float b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  // Test large numbers.
  { float a = 2147483647.0f;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("2.14748e+09", ss.str());
  }

  { double a = 22.3;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("22.3", ss.str());

    double b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  // Test precision.
  { double a = 22.345678;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("22.345678", ss.str());
  }

  { double a = 39.2408846427211;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("39.2408846427211", ss.str());
  }

  { double a = -119.943616083908;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("-119.943616083908", ss.str());
  }

  { size_t a = 22;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("22", ss.str());

    size_t b = 0;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestString) {
  { string a = "some_string";
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("\"some_string\"", ss.str());


    string b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { string a = "so}me{ : \n st'ring";
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("\"so}me{ : \\n st'ring\"", ss.str());

    string b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestArray) {
  { int a[2] = {1, 2};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[1,2]", ss.str());

    int b[2];
    JSONDeSerializationByType()(ss, &b);
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a[i], b[i]);
  }

  { string a[2] = {"aa", "bb"};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[\"aa\",\"bb\"]", ss.str());

    string b[2];
    JSONDeSerializationByType()(ss, &b);
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a[i], b[i]);
  }

  // Test array of size 1.
  // This is a hack for Expedia as their response JSONs do not always add []
  // in case of single element arrays.
  { int b[1];
    stringstream ss("1");
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(1, b[0]);
  }

  // Test indentation.
  { int a[2] = {1, 2};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("[\n 1,\n 2\n]", ss.str());

    int b[2];
    JSONDeSerializationByType()(ss, &b);
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a[i], b[i]);
  }
}

TEST(JSONSerializationByTypeTest, TestVector) {
  { vector<int> a = {1, 2};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[1,2]", ss.str());

    vector<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { vector<bool> a = {true, false};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[true,false]", ss.str());

    vector<bool> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { vector<string> a = {"aa", "bb"};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[\"aa\",\"bb\"]", ss.str());

    vector<string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  // Test vector of size 1.
  // This is a hack for Expedia as their response JSONs do not always add []
  // in case of single element arrays.
  { vector<int> b;
    stringstream ss("1");
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(1, b[0]);
  }

  // Test indentation.
  { vector<int> a = {1, 2};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("[\n 1,\n 2\n]", ss.str());

    vector<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestPair) {
  { pair<int, string> a = {1, "a"};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[1,\"a\"]", ss.str());

    pair<int, string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { pair<vector<int>, vector<string> > a = {{1, 2}, {"a", "b"}};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[[1,2],[\"a\",\"b\"]]", ss.str());

    pair<vector<int>, vector<string> > b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  // Test indentation.
  { pair<int, string> a = {1, "a"};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("[\n 1,\n \"a\"\n]", ss.str());

    pair<int, string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestSet) {
  { set<int> a = {1, 2};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[1,2]", ss.str());

    set<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { set<string> a = {"aa", "bb"};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[\"aa\",\"bb\"]", ss.str());

    set<string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  // Test indentation.
  { set<int> a = {1, 2};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("[\n 1,\n 2\n]", ss.str());

    set<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestUnorderedSet) {
  { unordered_set<int> a = {1};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[1]", ss.str());

    unordered_set<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unordered_set<string> a = {"aa"};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[\"aa\"]", ss.str());

    unordered_set<string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  // Test indentation.
  { unordered_set<int> a = {1};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("[\n 1\n]", ss.str());

    unordered_set<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestMap) {
  { map<int, string> a = {{1, "a"}, {2, "b"}};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[[1,\"a\"],[2,\"b\"]]", ss.str());


    map<int,string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { map<string, string> a = {{"A", "a"}, {"B", "b"}};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[[\"A\",\"a\"],[\"B\",\"b\"]]", ss.str());


    map<string,string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);

  }

  // Test indentation.
  { map<int, string> a = {{1, "a"}, {2, "b"}};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("[\n [\n  1,\n  \"a\"\n ],\n [\n  2,\n  \"b\"\n ]\n]", ss.str());

    map<int,string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestUnorderedMap) {
  { unordered_map<int, string> a = {{1, "a"}};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("[[1,\"a\"]]", ss.str());

    unordered_map<int,string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unordered_map<string, string> a = {{"A", "a"}};
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("{\"A\":\"a\"}", ss.str());

    unordered_map<string,string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
  // Test indentation.
  { unordered_map<int, string> a = {{1, "a"}};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("[\n [\n  1,\n  \"a\"\n ]\n]", ss.str());

    unordered_map<int,string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { unordered_map<string, string> a = {{"A", "a"}};
    stringstream ss;
    JSONSerializationByType(1)(ss, a);
    EXPECT_EQ("{\n \"A\": \"a\"\n}", ss.str());

    unordered_map<string,string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestUniquePointer) {
  { unique_ptr<int> a(new int(12));
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("12", ss.str());

    unique_ptr<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }

  { unique_ptr<string> a(new string("abcd"));
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("\"abcd\"", ss.str());

    unique_ptr<string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }
}

TEST(JSONSerializationByTypeTest, TestSharedPointer) {
  { shared_ptr<int> a(new int(12));
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("12", ss.str());

    shared_ptr<int> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }

  { shared_ptr<string> a(new string("abcd"));
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("\"abcd\"", ss.str());

    shared_ptr<string> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(*a, *b);
  }
}

TEST(JSONSerializationByTypeTest, TestDataWithZeroDefaults) {
  { TestDataWithSerialization<TestDataWithZeroDefaults> a;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ(string("0 \0  0", 6), ss.str());
  }

  { TestDataWithSerialization<TestDataWithZeroDefaults> a;
    a.test_int = 1;
    a.test_bool = true;
    a.test_char = 'B';
    a.test_str = "some_string";

    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("1 B some_string 1", ss.str());

    TestDataWithSerialization<TestDataWithZeroDefaults> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestDataWithCustomDefaults) {
  { TestDataWithSerialization<TestDataWithCustomDefaults> a;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("1 A some_string 1", ss.str());

    TestDataWithSerialization<TestDataWithCustomDefaults> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }

  { TestDataWithSerialization<TestDataWithCustomDefaults> a;
    a.test_int = 22;
    a.test_bool = false;
    a.test_char = 'F';
    a.test_str = "some_other_string";

    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("22 F some_other_string 0", ss.str());

    TestDataWithSerialization<TestDataWithCustomDefaults> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

TEST(JSONSerializationByTypeTest, TestDataWithSerializationDerived) {
  { TestDataWithSerializationDerived<TestDataWithZeroDefaults> a;
    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ(string("0 \0  0", 6), ss.str());
  }

  { TestDataWithSerializationDerived<TestDataWithZeroDefaults> a;
    a.test_int = 1;
    a.test_bool = true;
    a.test_char = 'B';
    a.test_str = "some_string";

    stringstream ss;
    JSONSerializationByType()(ss, a);
    EXPECT_EQ("1 B some_string 1", ss.str());

    TestDataWithSerializationDerived<TestDataWithZeroDefaults> b;
    JSONDeSerializationByType()(ss, &b);
    EXPECT_EQ(a, b);
  }
}

}  // namespace test
}  // namespace serial
