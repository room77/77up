/*
Copyright 2013, Room 77, Inc.
@author _FILLME_

Test for util
*/

#include "test/cc/unit_test.h"
#include "util/network/util.h"

TEST(UtilTest, TestParseCGIArgumentsUnstrict_Basic) {
  string args = "one=1&two=2";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_LeadingAmpersand) {
  string args = "&one=1&two=2";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_NoValue) {
  string args = "novalue&one=1&two=2";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_NoKey) {
  string args = "=nokey&one=1&two=2";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_MultipleEquals) {
  string args = "====&one=1&two=2";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_JustATerribleMess) {
  string args = "=nothing&=&&stuff&==&one=1&&=ohmygodthis?isterrible&==&two=2&=&";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_JustATerribleMessWithHash) {
  string args =
      "=nothing&=&&stuff&==&one=1&&=ohmygodthis?isterrible&==&two=2&=&#invalid1=2&invalid2=3";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_JustATerribleMessWithHashInTheMiddleOfValue) {
  string args = "one=1&two=2#random&three=3";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestParseCGIArgumentsUnstrict_JustATerribleMessWithHashInTheMiddleOfKey) {
  string args = "one=1&two=2&thr#ee=3&four=4";
  map<string, string> result;
  EXPECT_TRUE(NetworkUtil::ParseCGIArgumentsUnstrict(args, &result));
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result["one"], "1");
  EXPECT_EQ(result["two"], "2");
}

TEST(UtilTest, TestIsValidEmail) {
  // examples taken from Wikipedia
  // emails that are commented out should pass according to RFC 822 but don't
  vector<string> valid_emails = { "niceandsimple@example.com",
                                  "very.common@example.com",
                                  "a.little.lengthy.but.fine@dept.example.com",
                                  "disposable.style.email.with+symbol@example.com",
                                  //"user@[IPv6:2001:db8:1ff::a0b:dbd0]",
                                  //"\"much.more unusual\"@example.com",
                                  "\"very.unusual.@.unusual.com\"@example.com",
                                  //"\"very.(),:;<>[]\\\".VERY.\\\"very@\\\\ \\\"very\\\".unusual\"@strange.example.com",
                                  //"postbox@com", // (top-level domains are valid hostnames),
                                  //"admin@mailserver1", // (local domain name with no TLD),
                                  "!#$%&'*+-/=?^_`{}|~@example.org",
                                  //"\"()<>[]:,;@\\\\\\\"!#$%&'*+-/=?^_`{}| ~  ? ^_`{}|~.a\"@example.org",
                                  //"\" \"@example.org" // (space between the quotes)
  };

  vector<string> invalid_emails = { "Abc.example.com", // (an @ character must separate the local and domain parts)
                                    "Abc.@example.com", // (character dot(.) is last in local part)
                                    //"Abc..123@example.com", // (character dot(.) is double)
                                    "A@b@c@example.com", // (only one @ is allowed outside quotation marks)
                                    "a\"b(c)d,e:f;g<h>i[j\\k]l@example.com", // (none of the special characters in this local part are allowed outside quotation marks)
                                    "just\"not\"right@example.com", // (quoted strings must be dot separated, or the only element making up the local-part)
                                    "this is\"not\\allowed@example.com", // (spaces, quotes, and backslashes may only exist when within quoted strings and preceded by a backslash)
                                    "this\\ still\\\"not\\\\allowed@example.com" // (even if escaped (preceded by a backslash), spaces, quotes, and backslashes must still be contained by quotes)
  };

  for (const string& email : valid_emails) {
    EXPECT_TRUE(NetworkUtil::IsValidEmail(email)) << email;
  }
  for (const string& email : invalid_emails) {
    EXPECT_FALSE(NetworkUtil::IsValidEmail(email)) << email;
  }

}
