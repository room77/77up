// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test for JSON serialization and deserialization.

#include "util/serial/serializer_macros.h"

#include "test/cc/unit_test.h"
#include "util/serial/type_handlers/test_util.h"
#include "util/serial/utils/test_util.h"

namespace serial {
namespace test {

TEST(SerializerJSONTest, TestArithmetic) {
  { TestData<int> a(-12);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":-12}", str);

    TestData<int> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<int> a(12);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":12}", str);

    TestData<int> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unsigned int> a(22);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":22}", str);

    TestData<unsigned int> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<char> a('A');
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":65}", str);

    TestData<char> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<bool> a(true);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":true}", str);

    TestData<bool> b(false);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<long> a(-223);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":-223}", str);

    TestData<long> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unsigned long> a(223);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":223}", str);

    TestData<unsigned long> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<float> a(22.3f);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":22.3}", str);

    TestData<float> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<double> a(22.3);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":22.3}", str);

    TestData<double> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<size_t> a(22);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":22}", str);

    TestData<size_t> b(0);
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestString) {
  { TestData<string> a("some_string");
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":\"some_string\"}", str);

    TestData<string> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<string> a("open 9 AM \xE2\x80\x93 10 PM, but");
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":\"open 9 AM \\u2013 10 PM, but\"}", str);

    TestData<string> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<string> a(" some { : ] [ ]st\nring , ");
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":\" some { : ] [ ]st\\nring , \"}", str);

    TestData<string> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);

    // Test Parsing Errors.
    EXPECT_FALSE(b.FromJSON(str.substr(0, str.size() - 1)));
    EXPECT_FALSE(b.FromJSON(str.substr(0, str.size() - 3)));
  }
}

TEST(SerializerJSONTest, TestArray) {
  { TestData<int[2]> a({1, 2});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[1,2]}", str);

    TestData<int[2]> b({0, 0});
    EXPECT_TRUE(b.FromJSON(str));
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a.data[i], b.data[i]);
  }

  { TestData<string[2]> a({"aa", "bb"});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[\"aa\",\"bb\"]}", str);

    TestData<string[2]> b;
    EXPECT_TRUE(b.FromJSON(str));
    for (int i = 0; i < 2; ++i)
      EXPECT_EQ(a.data[i], b.data[i]);
  }
}

TEST(SerializerJSONTest, TestVector) {
  { TestData<vector<int>> a({1, 2});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[1,2]}", str);

    TestData<vector<int>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<vector<string>> a({"aa", "bb"});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[\"aa\",\"bb\"]}", str);

    TestData<vector<string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestPair) {
  { TestData<pair<int, string>> a({1, "a"});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[1,\"a\"]}", str);

    TestData<pair<int, string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<pair<vector<int>, vector<string>>> a({{1, 2}, {"a", "b"}});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[[1,2],[\"a\",\"b\"]]}", str);

    TestData<pair<vector<int>, vector<string>>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestSet) {
  { TestData<set<int>> a({1, 2});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[1,2]}", str);

    TestData<set<int>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<set<string>> a({"aa", "bb"});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[\"aa\",\"bb\"]}", str);

    TestData<set<string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestUnorderedSet) {
  { TestData<unordered_set<int>> a({1});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[1]}", str);

    TestData<unordered_set<int>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_set<string>> a({"aa"});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[\"aa\"]}", str);

    TestData<unordered_set<string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestMap) {
  { TestData<map<int, string>> a({{1, "a"}, {2, "b"}});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[[1,\"a\"],[2,\"b\"]]}", str);

    TestData<map<int,string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<map<string, string>> a({{"A", "a"}, {"B", "b"}});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[[\"A\",\"a\"],[\"B\",\"b\"]]}", str);

    TestData<map<string,string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestUnorderedMap) {
  { TestData<unordered_map<int, string>> a(unordered_map<int, string>{{1, "a"}});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":[[1,\"a\"]]}", str);

    TestData<unordered_map<int,string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unordered_map<string, string>> a(unordered_map<string, string>{{"A", "a"}});
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":{\"A\":\"a\"}}", str);

    TestData<unordered_map<string,string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestUniquePointer) {
  {
    TestData<unique_ptr<int>> a(12);
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":12}", str);

    TestData<unique_ptr<int>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<unique_ptr<string>> a("abcd");
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":\"abcd\"}", str);

    TestData<unique_ptr<string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestSharedPointer) {
  { TestData<shared_ptr<int>> a(shared_ptr<int>(new int(12)));
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":12}", str);

    TestData<shared_ptr<int>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(*a.data, *b.data);
  }

  { TestData<shared_ptr<string>> a(shared_ptr<string>(new string("abcd")));
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":\"abcd\"}", str);

    TestData<shared_ptr<string>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(*a.data, *b.data);
  }
}

TEST(SerializerJSONTest, TestDataWithZeroDefaults) {
  { TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> a;
    string str = a.ToJSON();
    EXPECT_EQ(string("{\"data\":0 \0  0}", 15), str);
  }

  { TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> a;
    a.data.test_int = 1;
    a.data.test_bool = true;
    a.data.test_char = 'B';
    a.data.test_str = "some_string";

    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":1 B some_string 1}", str);

    TestData<TestDataWithSerialization<TestDataWithZeroDefaults>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}


TEST(SerializerJSONTest, TestDataWithCustomDefaults) {
  { TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> a;
    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":1 A some_string 1}", str);

    TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }

  { TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> a;
    a.data.test_int = 22;
    a.data.test_bool = false;
    a.data.test_char = 'F';
    a.data.test_str = "some_other_string";

    string str = a.ToJSON();
    EXPECT_EQ("{\"data\":22 F some_other_string 0}", str);

    TestData<TestDataWithSerialization<TestDataWithCustomDefaults>> b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
}

TEST(SerializerJSONTest, TestDataWithAddedRemovedFields) {
  TestDataBasic a(true, 'A', 1, "some_string");
  string str = a.ToJSON();
  EXPECT_EQ("{\"test_bool\":true,\"test_char\":65,\"test_int\":1,"
      "\"test_str\":\"some_string\"}", str);
  TestDataBasic b;
  EXPECT_TRUE(b.FromJSON(str));
  EXPECT_EQ(a, b);

  LOG(INFO) << "Removed";

  TestDataFieldRemoved c;
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromJSON(str));
  EXPECT_EQ(c, a);

  TestDataFieldAdded d;
  EXPECT_EQ(-1, d.test_added);
  EXPECT_FALSE(d == b);
  EXPECT_TRUE(d.FromJSON(str));
  EXPECT_EQ(d, a);
  EXPECT_EQ(0, d.test_added);

  TestDataFieldAddedRemoved e;
  EXPECT_EQ(-1, e.test_added);
  EXPECT_FALSE(e == a);
  EXPECT_TRUE(e.FromJSON(str));
  EXPECT_EQ(e, a);
  EXPECT_EQ(0, e.test_added);
}

TEST(SerializerJSONTest, TestNestedData) {
  TestData<TestNested<TestDataBasic> > a;
  a.data.test_basic = { TestDataBasic(true, 'A', 1, "some_string"),
                        TestDataBasic(false, 'B', 23, "some_other_string")};
  string str = a.ToJSON({1});

  EXPECT_EQ("{\n"
            " \"data\": {\n"
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
            " }\n"
            "}", str);
  TestData<TestNested<TestDataBasic>> b;
  EXPECT_TRUE(b.FromJSON(str));
  EXPECT_EQ(a, b);

  TestData<TestNested<TestDataFieldRemoved>> c;
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromJSON(str));
  EXPECT_EQ(c, a);

  TestData<TestNested<TestDataFieldAdded>> d;
  EXPECT_FALSE(d == b);
  EXPECT_TRUE(d.FromJSON(str));
  EXPECT_EQ(d, a);

  TestData<TestNested<TestDataFieldAddedRemoved>> e;
  EXPECT_FALSE(e == a);
  EXPECT_TRUE(e.FromJSON(str));
  EXPECT_EQ(e, a);
}

TEST(SerializerJSONTest, TestDataWithCallbacks) {
  TestDataBasic a(true, 'A', 1, "some_string");
  string str = a.ToJSON();
  EXPECT_EQ("{\"test_bool\":true,\"test_char\":65,\"test_int\":1,"
      "\"test_str\":\"some_string\"}", str);
  TestDataBasic b;
  EXPECT_TRUE(b.FromJSON(str));
  EXPECT_EQ(a, b);

  TestDataWithCallback c;
  EXPECT_FALSE(c.callback);
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c.FromJSON(str));
  EXPECT_EQ(c, a);
  EXPECT_TRUE(c.callback);

  TestDataWithCallback d;
  d.return_value_on_callback = false;
  EXPECT_FALSE(d.callback);
  EXPECT_FALSE(d.FromJSON(str));
  EXPECT_TRUE(d.callback);
}

TEST(SerializerJSONTest, TestNull) {
  const string str =  "{\n"
                      " \"data\": {\n"
                      "  \"test_basic\": [\n"
                      "   {\n"
                      "    \"test_bool\": null,\n"
                      "    \"test_char\": null,\n"
                      "    \"test_int\": null,\n"
                      "    \"test_str\": null\n"
                      "   },\n"
                      "   {\n"
                      "    \"test_bool\": null,\n"
                      "    \"test_char\": null,\n"
                      "    \"test_int\": null,\n"
                      "    \"test_str\": null\n"
                      "   }\n"
                      "  ]\n"
                      " }\n"
                      "}";
  TestData<TestNested<TestDataBasic>> b;
  EXPECT_TRUE(b.FromJSON(str));
}

TEST(SerializerJSONTest, TestDataRequiredFields) {
  TestDataRequiredFields a(true, 'A', 1, "some_string");
  string str = a.ToJSON();
  EXPECT_EQ("{\"test_bool\":true,\"test_char\":65,\"test_int\":1,\"test_str\":\"some_string\"}",
            str);
  {
    TestDataBasic b;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);

    TestDataRequiredFields c;
    EXPECT_TRUE(b.FromJSON(str));
    EXPECT_EQ(a, b);
  }
  {
    // Remove the test_char field.
    string str = "{\"test_bool\":true,\"test_int\":1,\"test_str\":\"some_string\"}";
    TestDataBasic b;
    EXPECT_TRUE(b.FromJSON(str));

    TestDataRequiredFields c;
    EXPECT_FALSE(c.FromJSON(str));
  }
  {
    // Remove the test_int field.
    string str = "{\"test_bool\":true,\"test_char\":65,\"test_str\":\"some_string\"}";
    TestDataBasic b;
    EXPECT_TRUE(b.FromJSON(str));

    TestDataRequiredFields c;
    EXPECT_FALSE(c.FromJSON(str));
  }
}

TEST(SerializerJSONTest, TestVirtual) {
  TestDataVirtualDerived a;
  TestDataVirtualDerived b;
  a.test_int = 1;
  a.test_float = 2.0;
  const TestDataVirtualBase& c = a;
  EXPECT_NE(c.ToJSON().find("test_float"), string::npos);
  b.FromJSON(c.ToJSON());
  EXPECT_FLOAT_EQ(b.test_float, a.test_float);
}

TEST(SerializerJSONTest, TestPrettyName) {
  TestDataPrettyName a;
  a.test_bool = true;
  string str = a.ToJSON();
  EXPECT_EQ("{\"__type_info\":\"serial::test::TestDataPrettyName\",\"test_bool\":true}", str);
  TestDataPrettyName b;
  EXPECT_TRUE(b.FromJSON(str));
  EXPECT_EQ(a, b);
}

}  // namespace test
}  // namespace serial
