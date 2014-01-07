// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// File for encoding and decoding strings to JSON or Binary with UTF-8 support.

#ifndef _PUBLIC_UTIL_SERIAL_ENCODING_ENCODING_H_
#define _PUBLIC_UTIL_SERIAL_ENCODING_ENCODING_H_

#include <sstream>

#include "base/common.h"

namespace serial {
namespace encoding {

inline string TwoDigitHex(unsigned short c) {
  int h1, h2;
  h1 = c >> 4;
  h2 = c & 15;
  string s;
  s += (h1 < 10 ? ('0' + h1) : (h1 - 10 + 'A'));
  s += (h2 < 10 ? ('0' + h2) : (h2 - 10 + 'A'));
  return s;
}

inline string FourDigitHex(unsigned short c) {
  unsigned char h1 = c >> 8;
  unsigned char h2 = c & 255;
  return TwoDigitHex(h1) + TwoDigitHex(h2);
}

// Given a character, generates an equivalent unicode string.
inline string UnicodeEscaped(unsigned short c) {
  // special case: these code points do not seem cross-browser
  switch(c) {
    case 0x91: c = '`'; break;
    case 0x92: c = '\''; break;
    case 0x93: case 0x94: c = '"'; break;
    default : break;
  }
  return string("\\u") + FourDigitHex(c);
}

// Given unicode string, generates an equivalent character.
inline unsigned int UnicodeUnEscaped(const string& str) {
  unsigned int d = 0;
  try {
    if (str.find("\\u") == 0) {
      string temp = str.substr(2);
      d = std::stoul(temp, nullptr, 16);
    } else {
      d = std::stoul(str, nullptr, 16);
    }
  } catch(...) {
    ASSERT_DEV(false) << "Invalid unicode sequence : \\u" << str;
  }
  return d;
}

// Given a character, generates an equivalent octal string.
inline string OctalEscaped(unsigned short c) {
  // Output octal representation of c.
  string str(4, '\\');
  str[1] = '0' + (c >> 6);
  str[2] = '0' + ((c & 63) >> 3);
  str[3] = '0' + (c & 7);
  return str;
}

// Given unicode string, generates an equivalent character.
inline unsigned int OctalUnEscaped(const string& str) {
  unsigned int d = 0;
  try {
    if (str.find("\\") == 0) {
      string temp = str.substr(1);
      d = std::stoul(temp, nullptr, 8);
    } else {
      d = std::stoul(str, nullptr, 8);
    }
  } catch(...) {
    LOG(ERROR) << "Invalid octal sequence : \\" << str;
  }
  return d;
}

// JSON-encode a string of 8-bit characters that are not UTF-8 encoded.
inline string JSONEncode8Bit(const string& s) {
  string encoded;
  for (size_t i = 0; i < s.size(); i++) {
    unsigned char c = s[i];
    encoded += UnicodeEscaped(c);
  }
  return encoded;
}

// Given a multibyte UTF8 character starting at str, extracts at most len
// characters to construct unicode escaped hex digits.
// Returns the encoded string and the number of bytes extracted from the string.
pair<string, int> EncodeMultiByteUTF8Char(const char* str, int len);

// Decodes a UTF-8 codepoint and appends it to a string.
bool DecodeMultiByteUTf8CodePoint(unsigned int codepoint, string* res);

inline string DecodeMultiByteUTf8CodePoint(unsigned int codepoint) {
  string res;
  DecodeMultiByteUTf8CodePoint(codepoint, &res);
  return res;
}

// Dumps the escaped character to the stream.
void EscapeCharacter(ostream& out, const unsigned char c, bool json = false);

inline string EscapeCharacter(const unsigned char c, bool json = false) {
  stringstream ss;
  EscapeCharacter(ss, c, json);
  return !ss.fail() ? ss.str() : "";
}

// Fills the result with the next unescaped character from the stream.
// If expect_slash is false, we assume that '\' has already been consumed.
bool UnEscapeCharacter(istream& in, string* res, bool expect_slash = true);

inline bool UnEscapeCharacter(const string& str, string* res,
                              bool expect_slash = true) {
  stringstream ss(str);
  return UnEscapeCharacter(ss, res, expect_slash);
}

inline string UnEscapeCharacter(const string& str, bool expect_slash = true) {
  stringstream ss(str);
  string res;
  UnEscapeCharacter(ss, &res, expect_slash);
  return res;
}

// Writes a string to a stream after escaping all metacharacters and UTF-8
// characters correctly.
void EscapeStringToStream(ostream& out, const string& input, bool json = false);

inline string EscapeString(const string& input, bool json = false) {
  stringstream ss;
  EscapeStringToStream(ss, input, json);
  return !ss.fail() ? ss.str() : "";
}

// Reads a string from a stream after unescaping all metacharacters and UTF-8
// characters correctly. Returns true if the operation was successful and
// false otherwise.
bool UnEscapeStringFromStream(istream& in, string* res,
                              const string& delimiters = "");

inline bool UnEscapeString(const string& str, string* res,
                           const string& delimiters = "") {
  stringstream ss(str);
  return UnEscapeStringFromStream(ss, res, delimiters);
}

inline string UnEscapeString(const string& str,
                             const string& delimiters = "") {
  stringstream ss(str);
  string res;
  UnEscapeStringFromStream(ss, &res, delimiters);
  return res;
}

// Unescapes the string till the next '"' and eats the quote.
bool UnEscapeStringToNextQuote(istream& in, string* res);

// Reads a *possibly* quoted string to a stream after unescaping all
// metacharacters and UTF-8 characters correctly.
// This ignores any preceeding spaces till it finds the first valid character.
// If the first valid character is '"', then the string is parsed as quoted
// string and all characters are unescaped, otherwise the string is parsed upto
// the fallback delimiters. Returns true of the operation was successful and
// false otherwise.
bool UnEscapeQuotedStringFromStream(istream& in, string* res,
                                    const string& fallback_delimiters = " '\n",
                                    bool* was_quoted = nullptr);

inline bool UnEscapeQuotedString(const string& str, string* res,
                                 const string& delimiters = "",
                                 bool* was_quoted = nullptr) {
  stringstream ss(str);
  return UnEscapeQuotedStringFromStream(ss, res, delimiters, was_quoted);
}

inline string UnEscapeQuotedString(const string& str,
                                   const string& delimiters = "") {
  stringstream ss(str);
  string res;
  UnEscapeQuotedStringFromStream(ss, &res, delimiters);
  return res;
}

}  // namespace encoding
}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_ENCODING_ENCODING_H_
