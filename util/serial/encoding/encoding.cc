// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/encoding/encoding.h"

#include <cstring>
#include <istream>
#include <ostream>

#include "util/serial/utils/stream_util.h"
#include "util/templates/container_util.h"

namespace {

string* InitCharMappingLessThan32() {
  static string char_mapping[32];
  char_mapping['\b'] = "\\b";
  char_mapping['\f'] = "\\f";
  char_mapping['\n'] = "\\n";
  char_mapping['\r'] = "\\r";
  char_mapping['\t'] = "\\t";

  return char_mapping;
}

}  // namespace

namespace serial {
namespace encoding {

pair<string, int> EncodeMultiByteUTF8Char(const char* str, int len) {
  ASSERT_NOTNULL(str);
  string unencoded;
  unsigned int codepoint = 0;
  int expect_more = 0;

  int count = 0;
  while (*str != 0 && count < len) {
    const unsigned char c = *str;
    if (c < 128) break;

    ++count, ++str;
    unencoded.push_back(c);
    if (c >= 0xFE) {
      // invalid character in utf-8 -- error in input.
      break;
    } else if (c >= 0xFC) {   // 1111110x in binary
      // first byte of 6-byte utf-8 character
      codepoint = (c & 1);
      expect_more = 5;
    } else if (c >= 0xF8) {   // 111110xx in binary
      // first byte of 5-byte utf-8 character
      codepoint = (c & 3);
      expect_more = 4;
    } else if (c >= 0xF0) {   // 11110xxx in binary
      // first byte of 4-byte utf-8 character
      codepoint = (c & 7);
      expect_more = 3;
    } else if (c >= 0xE0) {   // 1110xxxx in binary
      // first byte of 3-byte utf-8 character
      codepoint = (c & 0xF);
      expect_more = 2;
    } else if (c >= 0xC0) {   // 110xxxxx in binary
      // first byte of 2-byte utf-8 character
      codepoint = (c & 0x1F);
      expect_more = 1;
    } else {
      // subsequent byte of a multi-byte utf-8 character : 10xxxxxx
      if (expect_more > 0) {
        codepoint = (codepoint << 6) + (c & 0x3F);
        if (--expect_more == 0) {
          unencoded.clear();
          break;
        }
      } else {
        VLOG(5) << "Unexpected character: " << c;
        break;
      }
    }
  }

  if (!count) return {"", count};

  string res;
  if (unencoded.size())
    res = JSONEncode8Bit(unencoded);
  else {
    if (codepoint <= 0xFFFF) {
      res = UnicodeEscaped(codepoint);
    } else {
      res = UnicodeEscaped(codepoint >> 16) + UnicodeEscaped(codepoint & 0xFFFF);
    }
  }
  return {res, count};
}

bool DecodeMultiByteUTf8CodePoint(unsigned int codepoint, string* res) {
  if (codepoint < 0x80) {
    res->push_back(codepoint & 0xFF);
  } else if (codepoint < 0x800) {
    res->push_back(0xC0 | ((codepoint >> 6) & 0xFF));
    res->push_back(0x80 | (codepoint & 0x3F));
  } else if (codepoint < 0x10000) {
    res->push_back(0xE0 | ((codepoint >> 12) & 0xFF));
    res->push_back(0x80 | ((codepoint >> 6) & 0x3F));
    res->push_back(0x80 | (codepoint & 0x3F));
  } else if (codepoint < 0x110000) {
    res->push_back(0xF0 | ((codepoint >> 18) & 0xFF));
    res->push_back(0x80 | ((codepoint >> 12) & 0x3F));
    res->push_back(0x80 | ((codepoint >> 6) & 0x3F));
    res->push_back(0x80 | (codepoint & 0x3F));
  } else {
    LOG(INFO) << "Codepoint: " << codepoint << " not supported!";
    return false;
  }
  return true;
}

// Optimization note. We tried escaping using an escaped array of size 256.
// That did not lead to any significant speedups and was slower in some cases.
// This seems to perform at par with it and is more deterministic in
// performance.
void EscapeCharacter(ostream& out, const unsigned char c, bool json) {
  // Handle the basic cases.
  if (c < 32) {
    static string* mapping = InitCharMappingLessThan32();
    const string& str = mapping[c];
    if (str.size()) out.write(str.c_str(), 2);
    else {
      if (json) {
        // Output hex representation of c.
        out << UnicodeEscaped(c);
      } else {
        // Output octal representation of c.
        out << OctalEscaped(c);
      }
    }
  } else {
    switch (c) {
      case '"': out.write("\\\"", 2); break;
      case '\\': out.write("\\\\", 2); break;
      default: out.put(c); break;
    }
  }
}

// Returns an unescaped character after extracting it from the stream.
// This function assumes that the previous character in the stream was '\\'.
bool UnEscapeCharacter(istream& in, string* res, bool expect_slash) {
  char ch = in.get();
  if (in.fail()) return false;

  if (expect_slash) {
    // Check if the character is \\ or not.
    if (ch != '\\') {
      res->push_back(ch);
      return true;
    }

    // Read the next character.
    ch = in.get();
    if (in.fail()) return false;
  }

  static const unordered_map<char, char> mappings = {{'\\', '\\'}, {'"', '"'},
    {'/', '/'}, {'b', '\b'}, {'f', '\f'}, {'n', '\n'}, {'r', '\r'}, {'t', '\t'}};

  char escaped_char = ::util::tl::FindWithDefault(mappings, ch, 0);
  if (escaped_char) {
    res->push_back(escaped_char);
    return true;
  }

  if (ch == 'u') {
    // Parse this as unicode (4 byte hex).
    string str(4, 0);
    in.read(const_cast<char*>(str.c_str()), 4);
    if (in.fail()) return 0;
    DecodeMultiByteUTf8CodePoint(UnicodeUnEscaped(str), res);
  } else {
    // Parse this as octal (3 byte octal).
    string str(3, ch);
    in.read(const_cast<char*>(str.c_str() + 1), 2);
    if (in.fail()) return false;
    res->push_back(OctalUnEscaped(str));
  }

  return true;
}

void EscapeStringToStream(ostream& out, const string& input, bool json) {
  out.put('"');
  const char* current = input.c_str();
  const int input_len = input.size();
  for (int i = 0; i < input_len; ++i, ++current) {
    unsigned char c = *current;
    if (c >= 128 && json) {
      // Check if the character is a special character.
      pair<string, int> p = EncodeMultiByteUTF8Char(current, input_len - i);
      out.write(p.first.c_str(), p.first.size());
      current += p.second - 1;
      i += p.second - 1;
      continue;
    }
    EscapeCharacter(out, c, json);
  }
  out.put('"');
}

bool UnEscapeStringFromStream(istream& in, string* res,
                              const string& delimiters) {
  ASSERT_NOTNULL(res);
  res->reserve(32);  // Optimization.
  while(true) {
    char ch = in.get();
    if (in.fail()) break;

    // Check if the character is one of the delimiters.
    if (delimiters.find(ch) != string::npos) {
      in.unget();
      break;
    }

    if (ch == '\\') UnEscapeCharacter(in, res, false);
    else res->push_back(ch);
  }
  res->shrink_to_fit();
  return !in.fail();
}

// Unescapes the string till the next '"' and eats the quote.
bool UnEscapeStringToNextQuote(istream& in, string* res) {
  ASSERT_NOTNULL(res);
  res->reserve(32);  // Optimization.
  while(true) {
    char ch = in.get();
    if (in.fail()) break;

    // Check if the character is a '"'.
    if (ch == '"') break;

    if (ch == '\\') UnEscapeCharacter(in, res, false);
    else res->push_back(ch);
  }
  res->shrink_to_fit();
  return !in.fail();
}

bool UnEscapeQuotedStringFromStream(istream& in, string* res,
                                    const string& fallback_delimiters,
                                    bool* was_quoted) {
  ASSERT_NOTNULL(res);
  // Skip spaces upto the first valid char.
  util::SkipSpaces(in);
  if (!in.good()) return false;

  char ch = in.get();
  if (in.fail()) return false;
  bool quoted = true;
  if (ch == '"') {
    UnEscapeStringToNextQuote(in, res);
  } else if (fallback_delimiters.size()) {
    in.unget();
    UnEscapeStringFromStream(in, res, fallback_delimiters);
    quoted = false;
  }

  if (was_quoted != nullptr) *was_quoted = quoted;
  return !in.fail();
}

}  // namespace encoding
}  // namespace serial
