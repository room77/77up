#include "base/common.h"

#include "strutil.h"
#include "test/cc/unit_test.h"

namespace strutil {
namespace test {

TEST(FromString, Sanity) {
  EXPECT_EQ(1, FromString<int>("1", 0));
  EXPECT_EQ(1, FromString<int>("", 1));

  EXPECT_EQ(1.1, FromString<double>("1.1", 0));
  EXPECT_EQ(1.1, FromString<double>("", 1.1));

  EXPECT_EQ("str", FromString<string>("str", ""));
  EXPECT_EQ("str", FromString<string>("", "str"));
}

TEST(EscapeString_C, Sanity) {
  string original = "a b\\c\r\n\b\f\t...\023<ab>& 1=00";
  string escaped = EscapeString_C(original);
  EXPECT_EQ(escaped, "\"a b\\\\c\\r\\n\\b\\f\\t...\\023<ab>& 1=00\"");
  string unescaped;
  EXPECT_EQ(UnescapeString_C(escaped, &unescaped), escaped.size());
  EXPECT_EQ(unescaped, original);
}

TEST(UnescapeString_CSV, Sanity) {
  string e1 = "\"hello world\"";
  string u1;
  EXPECT_EQ(UnescapeString_CSV(e1, &u1), e1.size());
  EXPECT_EQ(u1, "hello world");
  string e2 = "\"hello\"\"world\"";
  string u2;
  EXPECT_EQ(UnescapeString_CSV(e2, &u2), e2.size());
  EXPECT_EQ(u2, "hello\"world");
}

TEST(EscapeString_CGI, Sanity) {
  string original = "abc aa; <10>\ta";
  string e2 = EscapeString_CGI(original);
  EXPECT_EQ(e2, "abc+aa%3B+%3C10%3E%09a");
  EXPECT_EQ(UnescapeString_CGI(e2), original);
}

TEST(EscapeString_HTML, Sanity) {
  string original = "1 + 2 <= 3 && 5 > 4;";
  string escaped = EscapeString_HTML(original);
  EXPECT_EQ(escaped, "1 + 2 &lt;= 3 &amp;&amp; 5 &gt; 4;");
  EXPECT_EQ(UnescapeString_HTML(escaped), original);
}

TEST(EncodeString_Base64, Sanity) {
  string test_base64_original = "GWS/PCC1FU1:Optr---*";
  string base64_encoded = EncodeString_Base64(test_base64_original);
  EXPECT_EQ(base64_encoded, "R1dTL1BDQzFGVTE6T3B0ci0tLSo=");
  string decoded = DecodeString_Base64(base64_encoded);
  EXPECT_EQ(test_base64_original, decoded);

}

TEST(SameName, Sanity) {
  EXPECT_TRUE(SameName("hello_world", "HelloWorld"));
  EXPECT_FALSE(SameName("hello_world", "Hello1World"));
  EXPECT_FALSE(SameName("hello_world", "HelloWorld 0"));
}

TEST(EditDistance, Sanity) {
  EXPECT_EQ(EditDistance<string>("test", "test"), 0);
  EXPECT_EQ(EditDistance<string>("test", "tst"), 1);
  EXPECT_EQ(EditDistance<string>("test", "teest"), 1);
  EXPECT_EQ(EditDistance<string>("test", "teesT"), 2);
}

TEST(StripTags, Sanity) {
  EXPECT_EQ(StripTags("<b>foo</b> bar\n<img src='foo'>"), "foo bar\n");
  EXPECT_EQ(StripTags("foo </b> bar <a href='http://www.room77.com'/>"), "foo  bar ");
}

TEST(SameName, Tokenize) {
  EXPECT_EQ(Tokenize<vector<string> >("a b c d").size(), 4);
  EXPECT_EQ(Tokenize<set<string> >("a a b c").size(), 3);

  map<string, int> tokens = Tokenize<map<string, int> >("a a b c");
  EXPECT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens["a"], 2);
  EXPECT_EQ(tokens["b"], 1);
  EXPECT_EQ(tokens["c"], 1);
}

TEST(Trim, Sanity) {
  EXPECT_EQ(Trim("       "), "");
  EXPECT_EQ(Trim(" Foo Bar "), "Foo Bar");
}

TEST(Split, Sanity) {
  vector<string> tokens2;
  tokens2.clear();
  Split("one,,two", tokens2);
  EXPECT_EQ(tokens2.size(), 3);
  tokens2.clear();
  Split("one,,", tokens2);
  EXPECT_EQ(tokens2.size(), 3);
  tokens2.clear();
  Split("", tokens2);
  EXPECT_EQ(tokens2.size(), 1);
  tokens2.clear();
  Split(",", tokens2);
  EXPECT_EQ(tokens2.size(), 2);
}

TEST(ReplaceJunkMultiByteChars, Sanity) {
  EXPECT_EQ("\xC3\xA2\xE2\x82\xAC\xE2\x80\x9C", "â€“");
  EXPECT_EQ("“", ReplaceJunkMultiByteChars("â€œ"));
  EXPECT_EQ("9 AM—10 PM", ReplaceJunkMultiByteChars("9 AMâ€“10 PM"));
  EXPECT_EQ("9 AM—10 PM", ReplaceJunkMultiByteChars("9 AMâ€“10 PM"));
  EXPECT_EQ("abc陽陰deī'fgüãéhi-jk-lm‘”no“’pq–r—s…ètuävw'阴xy阳z",
      ReplaceJunkMultiByteChars("abcé™½é™°deÄ«Â´fgÃ¼Ã¢Ã©hiâ€¢jkâ€\"l"
          "mâ€˜â€\x9dnoâ€œâ€™pqâ€”râ€“sâ€¦Ã¨tuÃ\"vwÃ,Ãé˜´xyé˜³z"));
}

}  // namespace test
}  // namespace StrUtil
