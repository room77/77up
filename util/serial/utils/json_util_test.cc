// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/utils/json_util.h"

#include <sstream>

#include "test/cc/test_main.h"

namespace serial {
namespace util {
namespace test {

TEST(JSONSkipAheadField, Sanity) {
  string base_str = R"({[{ 1 : "value"}, { key : "value"} ], "key2" : 123213, []})";
  istringstream in(base_str);

  JSONSkipAheadField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(base_str.size(), in.tellg());

  // The string is at EOF, One more get should cause error.
  in.get();
  EXPECT_TRUE(in.eof());
  EXPECT_FALSE(in.good());
}

TEST(JSONSkipAheadField, InvalidStream) {
  istringstream in;
  JSONSkipAheadField(in);
  EXPECT_FALSE(in.good());
  EXPECT_TRUE(in.eof());
}

TEST(JSONSkipAheadField, BeginWithBrace1) {
  string part1 = R"([{ 1 : "value"}, { key : "value"} ])";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  JSONSkipAheadField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
}

TEST(JSONSkipAheadField, BeginWithComma) {
  string part1 = " ,";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  JSONSkipAheadField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
}

TEST(JSONSkipAheadField, BeginWithColon) {
  string part1 = " :";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  JSONSkipAheadField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
}

TEST(JSONSkipAheadField, StringWithotQuotes) {
  string part1 = "value";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  JSONSkipAheadField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
}

TEST(JSONSkipAheadField, StringWithQuotes) {
  string part1 = R"("value")";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  JSONSkipAheadField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
}

TEST(JSONSkipAheadAndReturnField, Sanity) {
  string base_str = R"({[{ 1 : "value"}, { key : "value"} ], "key2" : 123213, []})";
  istringstream in(base_str);

  string res = JSONSkipAheadAndReturnField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(base_str.size(), in.tellg());
  EXPECT_EQ(base_str, res);

  // The string is at EOF, One more get should cause error.
  in.get();
  EXPECT_TRUE(in.eof());
  EXPECT_FALSE(in.good());
}

TEST(JSONSkipAheadAndReturnField, InvalidStream) {
  istringstream in;
  string res = JSONSkipAheadAndReturnField(in);
  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_TRUE(res.empty());

  // The string is at EOF, One more get should cause error.
  in.get();
  EXPECT_TRUE(in.eof());
  EXPECT_FALSE(in.good());
}

TEST(JSONSkipAheadAndReturnField, BeginWithBrace1) {
  string part1 = R"([{ 1 : "value"}, { key : "value"} ])";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  string res = JSONSkipAheadAndReturnField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
  EXPECT_EQ(part1, res);
}

TEST(JSONSkipAheadAndReturnField, BeginWithComma) {
  string part1 = " ,";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  string res = JSONSkipAheadAndReturnField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
  EXPECT_EQ(part1, res);
}

TEST(JSONSkipAheadAndReturnField, BeginWithColon) {
  string part1 = " :";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  string res = JSONSkipAheadAndReturnField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
  EXPECT_EQ(part1, res);
}

TEST(JSONSkipAheadAndReturnField, StringWithotQuotes) {
  string part1 = "value";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  string res = JSONSkipAheadAndReturnField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
  EXPECT_EQ(part1, res);
}

TEST(JSONSkipAheadAndReturnField, StringWithQuotes) {
  string part1 = R"("value")";
  string part2 = R"(, "key2" : 123213, [])";
  istringstream in(part1 + part2);

  string res = JSONSkipAheadAndReturnField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part1.size(), in.tellg());
  EXPECT_EQ(part1, res);
}

TEST(JSONSkipAheadAndReturnField, TestUnescapedSimpleStringReachingEOF) {
  string part = "null";
  istringstream in(part);

  string res = JSONSkipAheadAndReturnField(in);

  EXPECT_TRUE(in.good());
  EXPECT_FALSE(in.eof());
  EXPECT_EQ(part.size(), in.tellg());
  EXPECT_EQ(part, res);
}

}  // namespace test
}  // namespace util
}  // namespace serial
