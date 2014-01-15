// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <sstream>

#include "util/serial/encoding/encoding.h"

#include "test/cc/test_main.h"

namespace serial {
namespace encoding {
namespace test {

TEST(EncodingTest, TwoDigitHex) {
  EXPECT_EQ("00", TwoDigitHex(0));
  EXPECT_EQ("41", TwoDigitHex('A'));
  EXPECT_EQ("7F", TwoDigitHex(127));
  EXPECT_EQ("FF", TwoDigitHex(255));
}

TEST(EncodingTest, FourDigitHex) {
  EXPECT_EQ("0000", FourDigitHex(0));
  EXPECT_EQ("0041", FourDigitHex('A'));
  EXPECT_EQ("007F", FourDigitHex(127));
  EXPECT_EQ("00FF", FourDigitHex(255));
  EXPECT_EQ("03FF", FourDigitHex(1023));
}

TEST(EncodingTest, UnicodeEscaped) {
  EXPECT_EQ("\\u0000", UnicodeEscaped(0));
  EXPECT_EQ("\\u0041", UnicodeEscaped('A'));
  EXPECT_EQ("\\u007F", UnicodeEscaped(127));
  EXPECT_EQ("\\u00FF", UnicodeEscaped(255));
  EXPECT_EQ("\\u03FF", UnicodeEscaped(1023));
  EXPECT_EQ("\\u0060", UnicodeEscaped(0x91));
  EXPECT_EQ("\\u0027", UnicodeEscaped(0x92));
  EXPECT_EQ("\\u0022", UnicodeEscaped(0x93));
  EXPECT_EQ("\\u0022", UnicodeEscaped(0x94));
  EXPECT_EQ("\\u00E8", UnicodeEscaped(u'è'));
  EXPECT_EQ("\\u2013", UnicodeEscaped(u'–'));
}

TEST(EncodingTest, UnicodeUnEscaped) {
  EXPECT_EQ(0, UnicodeUnEscaped(UnicodeEscaped(0)));
  EXPECT_EQ('A', UnicodeUnEscaped(UnicodeEscaped('A')));
  EXPECT_EQ(127, UnicodeUnEscaped(UnicodeEscaped(127)));
  EXPECT_EQ(255, UnicodeUnEscaped(UnicodeEscaped(255)));
  EXPECT_EQ(1023, UnicodeUnEscaped(UnicodeEscaped(1023)));

  EXPECT_EQ(0, UnicodeUnEscaped("0000"));
  EXPECT_EQ('A', UnicodeUnEscaped("0041"));
  EXPECT_EQ(127, UnicodeUnEscaped("007F"));
  EXPECT_EQ(255, UnicodeUnEscaped("00FF"));
  EXPECT_EQ(1023, UnicodeUnEscaped("03FF"));

  EXPECT_EQ(127, UnicodeUnEscaped("007f"));
  EXPECT_EQ(255, UnicodeUnEscaped("00ff"));
  EXPECT_EQ(1023, UnicodeUnEscaped("03ff"));

  EXPECT_EQ(u'è', UnicodeUnEscaped(UnicodeEscaped(u'è')));
}

TEST(EncodingTest, OctalEscaped) {
  EXPECT_EQ("\\000", OctalEscaped(0));
  EXPECT_EQ("\\101", OctalEscaped('A'));
  EXPECT_EQ("\\177", OctalEscaped(127));
  EXPECT_EQ("\\377", OctalEscaped(255));
  EXPECT_EQ("\\777", OctalEscaped(511));
}

TEST(EncodingTest, OctalUnEscaped) {
  EXPECT_EQ(0, OctalUnEscaped(OctalEscaped(0)));
  EXPECT_EQ('A', OctalUnEscaped(OctalEscaped('A')));
  EXPECT_EQ(127, OctalUnEscaped(OctalEscaped(127)));
  EXPECT_EQ(255, OctalUnEscaped(OctalEscaped(255)));
  EXPECT_EQ(511, OctalUnEscaped(OctalEscaped(511)));

  EXPECT_EQ(0, OctalUnEscaped("000"));
  EXPECT_EQ('A', OctalUnEscaped("101"));
  EXPECT_EQ(127, OctalUnEscaped("177"));
  EXPECT_EQ(255, OctalUnEscaped("377"));
  EXPECT_EQ(511, OctalUnEscaped("777"));
}

TEST(EncodingTest, JSONEncode8Bit) {
  EXPECT_EQ("", JSONEncode8Bit(""));
  EXPECT_EQ("\\u0041", JSONEncode8Bit("A"));
  EXPECT_EQ("\\u0060\\u00FF\\u00CC\\u0053", JSONEncode8Bit("\x91\xFF\xCCS"));
}

TEST(EncodingTest, EncodeMultiByteUTF8Char) {

  { pair<string, int> res = EncodeMultiByteUTF8Char("ABC", 3);
    EXPECT_EQ("", res.first);
    EXPECT_EQ(0, res.second);
  }

  { pair<string, int> res = EncodeMultiByteUTF8Char("\xA5""ABC", 4);
    EXPECT_EQ("\\u00A5", res.first);
    EXPECT_EQ(1, res.second);
  }

  // Test 2 byte UTF-8 char fail.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xC8""A", 2);
    EXPECT_EQ("\\u00C8", res.first);
    EXPECT_EQ(1, res.second);
  }

  // Test 2 byte UTF-8 char success.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xC8\x8A", 2);
    EXPECT_EQ("\\u020A", res.first);
    EXPECT_EQ(2, res.second);
    EXPECT_EQ("\u020A", "\xC8\x8A");
  }

  // Test 3 byte UTF-8 char fail.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xE8\x8A\x8B", 2);
    EXPECT_EQ("\\u00E8\\u008A", res.first);
    EXPECT_EQ(2, res.second);
  }

  // Test 3 byte UTF-8 char success.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xE8\x8A\x8B", 3);
    EXPECT_EQ("\\u828B", res.first);
    EXPECT_EQ(3, res.second);
    EXPECT_EQ("\u828B", "\xE8\x8A\x8B");
  }

  // Test 3 byte UTF-8 char success.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xE2\x80\x93", 3);
    EXPECT_EQ("\\u2013", res.first);
    EXPECT_EQ(3, res.second);
    EXPECT_EQ("\u2013", "\xE2\x80\x93");
  }

  // Test 4 byte UTF-8 char fail.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xF4\x81\x82\x83", 2);
    EXPECT_EQ("\\u00F4\\u0081", res.first);
    EXPECT_EQ(2, res.second);
  }

  // Test 4 byte UTF-8 char success.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xF4\x81\x82\x83", 4);
    EXPECT_EQ("\\u0010\\u1083", res.first);
    EXPECT_EQ(4, res.second);

    EXPECT_EQ("\U00101083", "\xF4\x81\x82\x83");
  }

  // Test 5 byte UTF-8 char fail.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xFA\x81\x82\x83\x84", 2);
    EXPECT_EQ("\\u00FA\\u0081", res.first);
    EXPECT_EQ(2, res.second);
  }

  // Test 5 byte UTF-8 char success.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xFA\x81\x82\x83\x84", 5);
    EXPECT_EQ("\\u0204\\u20C4", res.first);
    EXPECT_EQ(5, res.second);
    // TODO(pramodg,oztekin): This causes "invalid universal character error"
    // in clang. Investigate more. Disable for now.
    //EXPECT_EQ("\U020420C4", "\xFA\x81\x82\x83\x84");
  }

  // Test 6 byte UTF-8 char fail.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xFD\x81\x82\x83\x84\x85", 2);
    EXPECT_EQ("\\u00FD\\u0081", res.first);
    EXPECT_EQ(2, res.second);
  }

  // Test 6 byte UTF-8 char success.
  { pair<string, int> res = EncodeMultiByteUTF8Char("\xFD\x81\x82\x83\x84\x85", 6);
    EXPECT_EQ("\\u4108\\u3105", res.first);
    EXPECT_EQ(6, res.second);
    // TODO(pramodg,oztekin): This causes "invalid universal character error"
    // in clang. Investigate more. Disable for now.
    //EXPECT_EQ("\U41083105", "\xFD\x81\x82\x83\x84\x85");
  }
}

TEST(EncodingTest, EscapeCharacter) {
  EXPECT_EQ("\\000", EscapeCharacter(0));
  EXPECT_EQ("C", EscapeCharacter('C'));
  EXPECT_EQ("\\n", EscapeCharacter('\n'));
  EXPECT_EQ("\\\\", EscapeCharacter('\\'));
  EXPECT_EQ("\\036", EscapeCharacter(30, false));
  EXPECT_EQ("\\u001E", EscapeCharacter(30, true));
}

TEST(EncodingTest, EscapeCharacterTiming) {
  ::test::BenchMark<> b("EscapeCharacter (ms)");
  for (int i =0; i < 1000000; ++i) {
    EXPECT_EQ("C", EscapeCharacter('C'));
  }
}

TEST(EncodingTest, UnEscapeCharacter) {
  EXPECT_EQ(string("\0", 1), UnEscapeCharacter(EscapeCharacter(0)));
  EXPECT_EQ("C", UnEscapeCharacter(EscapeCharacter('C')));
  EXPECT_EQ("\n", UnEscapeCharacter(EscapeCharacter('\n')));
  EXPECT_EQ("\\", UnEscapeCharacter(EscapeCharacter('\\')));
  EXPECT_EQ("\x1E", UnEscapeCharacter(EscapeCharacter(30, false)));
  EXPECT_EQ("\x1E", UnEscapeCharacter(EscapeCharacter(30, true)));
}

TEST(EncodingTest, EscapeString) {
  EXPECT_EQ("\"\"", EscapeString(""));
  EXPECT_EQ("\"ABCD\"", EscapeString("ABCD"));
  EXPECT_EQ("\"AB\xC8\x8A \\basd\\f\\nas\\rasd \\t \\\\ \\\" / ABC\"",
            EscapeString("AB\xC8\x8A \basd\f\nas\rasd \t \\ \" / ""ABC"));

  EXPECT_EQ("\"Sofitel Fès Palais Jamaï\"",
            EscapeString("Sofitel Fès Palais Jamaï"));

  EXPECT_EQ("\"Sofitel F\\u00E8s Palais Jama\\u00EF\"",
      EscapeString("Sofitel Fès Palais Jamaï", true));
}

TEST(EncodingTest, UnEscapeQuotedString) {
  EXPECT_EQ("", UnEscapeQuotedString(EscapeString("")));
  EXPECT_EQ("ABCD", UnEscapeQuotedString(EscapeString("ABCD")));
  EXPECT_EQ("AB\xC8\x8A \basd\f\nas\rasd \t \\ \" / ""ABC",
            UnEscapeQuotedString(EscapeString(
                "AB\xC8\x8A \basd\f\nas\rasd \t \\ \" / ""ABC")));
  EXPECT_EQ("Sofitel Fès Palais Jamaï",
            UnEscapeQuotedString(EscapeString("Sofitel Fès Palais Jamaï")));
  EXPECT_EQ("è", UnEscapeQuotedString(EscapeString("è")));

  EXPECT_EQ("", UnEscapeQuotedString(EscapeString("", true)));
  EXPECT_EQ("ABCD", UnEscapeQuotedString(EscapeString("ABCD", true)));
  EXPECT_EQ("AB\xC8\x8A \basd\f\nas\rasd \t \\ \" / ""ABC",
            UnEscapeQuotedString(EscapeString(
                "AB\xC8\x8A \basd\f\nas\rasd \t \\ \" / ""ABC", true)));
  EXPECT_EQ("è", UnEscapeQuotedString(EscapeString("è", true)));
  EXPECT_EQ("Sofitel Fès Palais Jamaï",
            UnEscapeQuotedString(EscapeString("Sofitel Fès Palais Jamaï",
                                              true)));
}

}  // namespace test
}  // namespace encoding
}  // namespace serial
