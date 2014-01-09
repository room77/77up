// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/types/arbit_blob.h"

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "test/cc/test_main.h"
#include "util/serial/type_handlers/test_util.h"
#include "util/serial/utils/test_util.h"

namespace serial {
namespace types {
namespace test {

using namespace ::serial::test;  // NOLINT

TEST(ArbitBlobTest, TestArithmetic) {
  { TestData<int> a(12);
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("12", blob.data.str);
    TestData<int> b(0);
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<char> a('A');
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("65", blob.data.str);
    TestData<char> b(0);
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<bool> a(true);
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("true", blob.data.str);
    TestData<bool> b(false);
    LOG(INFO) << "[" << blob.ToJSON() << "]";
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<float> a(22.3f);
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("22.3", blob.data.str);
    TestData<float> b(false);
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<double> a(22.3);
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("22.3", blob.data.str);
    TestData<double> b(false);
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<size_t> a(22);
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("22", blob.data.str);
    TestData<size_t> b(false);
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestString) {
  { TestData<string> a("some_string");
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("\"some_string\"", blob.data.str);
    TestData<string> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<string> a("open 9 AM \xE2\x80\x93 10 PM, but");
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("\"open 9 AM \\u2013 10 PM, but\"", blob.data.str);
    TestData<string> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<string> a(" some { : ] [ ]st\nring , ");
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("\" some { : ] [ ]st\\nring , \"", blob.data.str);
    TestData<string> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestArray) {
  { TestData<int[2]> a({1, 2});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[1,2]", blob.data.str);
    TestData<int[2]> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<string[2]> a({"aa", "bb"});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[\"aa\",\"bb\"]", blob.data.str);
    TestData<string[2]> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestVector) {
  { TestData<vector<int>> a({1, 2});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[1,2]", blob.data.str);
    TestData<vector<int>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<vector<string>> a({"aa", "bb"});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[\"aa\",\"bb\"]", blob.data.str);
    TestData<vector<string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestPair) {
  { TestData<pair<int, string>> a({1, "a"});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[1,\"a\"]", blob.data.str);
    TestData<pair<int, string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<pair<vector<int>, vector<string>>> a({{1, 2}, {"a", "b"}});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[[1,2],[\"a\",\"b\"]]", blob.data.str);
    TestData<pair<vector<int>, vector<string>>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestSet) {
  { TestData<set<int>> a({1, 2});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[1,2]", blob.data.str);
    TestData<set<int>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<set<string>> a({"aa", "bb"});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[\"aa\",\"bb\"]", blob.data.str);
    TestData<set<string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestUnorderedSet) {
  { TestData<unordered_set<int>> a({1});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[1]", blob.data.str);
    TestData<unordered_set<int>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_set<string>> a({"aa"});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[\"aa\"]", blob.data.str);
    TestData<unordered_set<string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestMap) {
  { TestData<map<int, string>> a({{1, "a"}, {2, "b"}});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[[1,\"a\"],[2,\"b\"]]", blob.data.str);
    TestData<map<int, string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<map<string, string>> a({{"A", "a"}, {"B", "b"}});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[[\"A\",\"a\"],[\"B\",\"b\"]]", blob.data.str);
    TestData<map<string, string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestUnorderedMap) {
  { TestData<unordered_map<int, string>> a(unordered_map<int, string>{{1, "a"}});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("[[1,\"a\"]]", blob.data.str);
    TestData<unordered_map<int, string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_map<string, string>> a(unordered_map<string, string>{{"A", "a"}});
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("{\"A\":\"a\"}", blob.data.str);
    TestData<unordered_map<string, string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestUniquePointer) {
  { TestData<unique_ptr<int>> a(12);
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("12", blob.data.str);
    TestData<unique_ptr<int>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }

  { TestData<unique_ptr<string>> a("abcd");
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("\"abcd\"", blob.data.str);
    TestData<unique_ptr<string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(a, b);
  }
}

TEST(ArbitBlobTest, TestSharedPointer) {
  { TestData<shared_ptr<int>> a(shared_ptr<int>(new int(12)));
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("12", blob.data.str);
    TestData<shared_ptr<int>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(*a.data, *b.data);
  }

  { TestData<shared_ptr<string>> a(shared_ptr<string>(new string("abcd")));
    TestData<ArbitBlob> blob;
    EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
    EXPECT_EQ("\"abcd\"", blob.data.str);
    TestData<shared_ptr<string>> b;
    EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
    EXPECT_EQ(*a.data, *b.data);
  }
}

// Unfortunately custom structs cannot be parsed with this mechanism.
TEST(ArbitBlobTest, TestDataWithZeroDefaults) {
  TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> a;
  a.data.test_int = 1;
  a.data.test_bool = true;
  a.data.test_char = 'B';
  a.data.test_str = "some_string";

  TestData<ArbitBlob> blob;
  EXPECT_FALSE(blob.FromJSON(a.ToJSON()));
  EXPECT_EQ("1", blob.data.str);
  TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> b;
  EXPECT_FALSE(b.FromJSON(blob.ToJSON()));
}

TEST(ArbitBlobTest, TestDataWithAddedRemovedFields) {
  TestData<TestDataBasic> a(TestDataBasic(true, 'A', 1, "some_string"));

  TestData<ArbitBlob> blob;
  EXPECT_TRUE(blob.FromJSON(a.ToJSON()));
  EXPECT_EQ("{\"test_bool\":true,\"test_char\":65,\"test_int\":1,"
      "\"test_str\":\"some_string\"}", blob.data.str);
  TestData<TestDataBasic> b;
  EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
  EXPECT_EQ(a, b);

  TestData<TestDataFieldRemoved> c;
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromJSON(blob.ToJSON()));
  EXPECT_EQ(c, a);

  TestData<TestDataFieldAdded> d;
  EXPECT_EQ(-1, d.data.test_added);
  EXPECT_FALSE(d == b);
  EXPECT_TRUE(d.FromJSON(blob.ToJSON()));
  EXPECT_EQ(d, a);
  EXPECT_EQ(0, d.data.test_added);

  TestData<TestDataFieldAddedRemoved> e;
  EXPECT_EQ(-1, e.data.test_added);
  EXPECT_FALSE(e == a);
  EXPECT_TRUE(e.FromJSON(blob.ToJSON()));
  EXPECT_EQ(e, a);
  EXPECT_EQ(0, e.data.test_added);
}

TEST(ArbitBlobTest, TestNestedData) {
  TestData<TestNested<TestDataBasic>> a;
  a.data.test_basic = { TestDataBasic(true, 'A', 1, "some_string"),
                        TestDataBasic(false, 'B', 23, "some_other_string")};

  TestData<ArbitBlob> blob;
  EXPECT_TRUE(blob.FromJSON(a.ToJSON({1})));
  EXPECT_EQ("{\n"
            "  \"test_basic\": [\n"
            "   {\n"
            "    \"test_bool\": true,\n"
            "    \"test_char\": 65,\n"
            "    \"test_int\": 1,\n"
            "    \"test_str\": \"some_string\"\n"
            "   },\n"
            "   {\n"
            "    \"test_bool\": false,\n"
            "    \"test_char\": 66,\n"
            "    \"test_int\": 23,\n"
            "    \"test_str\": \"some_other_string\"\n"
            "   }\n"
            "  ]\n"
            " }", blob.data.str);

  TestData<TestNested<TestDataBasic>> b;
  EXPECT_TRUE(b.FromJSON(blob.ToJSON()));
  EXPECT_EQ(a, b);

  TestData<TestNested<TestDataFieldRemoved>> c;
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromJSON(blob.ToJSON()));
  EXPECT_EQ(c, a);

  TestData<TestNested<TestDataFieldAdded>> d;
  EXPECT_FALSE(d == b);
  EXPECT_TRUE(d.FromJSON(blob.ToJSON()));
  EXPECT_EQ(d, a);

  TestData<TestNested<TestDataFieldAddedRemoved>> e;
  EXPECT_FALSE(e == a);
  EXPECT_TRUE(e.FromJSON(blob.ToJSON()));
  EXPECT_EQ(e, a);
}

TEST(ArbitBlobTest, TestEmptyDirect) {
  { ArbitBlob a, b;
    EXPECT_EQ("null", a.ToJSON());
    EXPECT_TRUE(b.FromJSON(a.ToJSON()));
    EXPECT_EQ("", a.str);
    EXPECT_EQ("", b.str);
  }
}

TEST(ArbitBlobTest, TestEmptyInsideAnotherStruct) {
  { TestData<ArbitBlob> a, b;
    EXPECT_EQ("{\"data\":null}", a.ToJSON());
    EXPECT_TRUE(b.FromJSON(a.ToJSON()));
    EXPECT_EQ("", a.data.str);
    EXPECT_EQ("", b.data.str);
  }
}

TEST(ArbitBlobTest, TestConstructor) {
  ArbitBlob blob("some_string");
  EXPECT_EQ("some_string", blob.str);
}

}  // namespace test
}  // namespace types
}  // namespace serial
