// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/utils/stream_util.h"

#include <chrono>
#include <sstream>
#include <unordered_set>

#include "test/cc/unit_test.h"

namespace serial {
namespace util {
namespace test {

TEST(StreamUtilTest, SkipSpaces) {
  stringstream  ss("{");
  ::test::BenchMark<> b("SkipSpaces (ms)");
  for (int i =0; i < 1000000; ++i) {
    ss.seekg(ss.beg);
    SkipSpaces(ss);
    EXPECT_EQ('{', ss.get());
  }
}

TEST(StreamUtilTest, SkipToNextChar) {
  stringstream  ss("          {");
  ::test::BenchMark<> b("SkipToNextChar (ms)");
  for (int i =0; i < 100000; ++i) {
    ss.seekg(ss.beg);
    EXPECT_EQ('{' , SkipToNextChar(ss));
  }
}

TEST(StreamUtilTest, ExtractDelimited) {
  stringstream  ss("asdasd;kljgdsjlajdsjk,jhsdfjkhsdf");
  ::test::BenchMark<> b("SkipSpaces (ms)");
  for (int i =0; i < 100000; ++i) {
    ss.seekg(ss.beg);
    EXPECT_EQ("asdasd;kljgdsjlajdsjk", ExtractDelimited(ss, " \n,"));
  }
}

TEST(StreamUtilTest, ExtractDelimitedEnd) {
  stringstream  ss("asdasd");
  ::test::BenchMark<> b("ExtractDelimitedEnd (ms)");
  for (int i =0; i < 1; ++i) {
    ss.seekg(ss.beg);
    EXPECT_EQ("asdasd", ExtractDelimited(ss, " \n,"));
  }
}

TEST(StreamUtilTest, SkipDelimitedString) {
  stringstream  ss("asdasd;\\,kljgdsjlajdsjk,jhsdfjkhsdf");
  ::test::BenchMark<> b("SkipDelimitedString (ms)");
  for (int i =0; i < 1; ++i) {
    ss.seekg(ss.beg);
    SkipDelimitedString(ss, " \n,");
    EXPECT_EQ(',', ss.get());
    EXPECT_EQ(9, ss.tellg());
  }
}

TEST(StreamUtilTest, SkipDelimitedStringSkipEscaped) {
  stringstream  ss("asdasd;\\,kljgdsjlajdsjk,jhsdfjkhsdf");
  ::test::BenchMark<> b("SkipDelimitedStringSkipEscaped (ms)");
  for (int i =0; i < 100000; ++i) {
    ss.seekg(ss.beg);
    SkipDelimitedString(ss, " \n,", true);
    EXPECT_EQ(',', ss.get());
    EXPECT_EQ(24, ss.tellg());
  }
}

TEST(StreamUtilTest, SkipQuotedString) {
  stringstream  ss("\"asdasd;\\\"kljgdsjlajdsjk\"jhsdfjkhsdf\"");
  ::test::BenchMark<> b("SkipQuotedString (ms)");
  for (int i =0; i < 100000; ++i) {
    ss.seekg(ss.beg);
    SkipQuotedString(ss, "");
    EXPECT_EQ('j', ss.get());
    EXPECT_EQ(26, ss.tellg());
  }
}

TEST(StreamUtilTest, ExpectNext) {
  stringstream  ss("    {");
  ::test::BenchMark<> b("ExpectNext (ms)");
  for (int i =0; i < 10000; ++i) {
    ss.seekg(ss.beg);
    EXPECT_EQ('{', ExpectNext(ss, ",{"));
  }
}

TEST(StreamUtilTest, ExtractQuotedString) {
  stringstream  ss("\"asdasd;\"kljgdsjlajdsjk\"jhsdfjkhsdf\"");
  ::test::BenchMark<> b("ExtractQuotedString (ms)");
  for (int i =0; i < 100000; ++i) {
    ss.seekg(ss.beg);
    string res;
    ExtractQuotedString(ss, &res);
    EXPECT_EQ("asdasd;", res);
  }
}

}  // namespace test
}  // namespace util
}  // namespace serial
