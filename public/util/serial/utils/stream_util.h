// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Utility file for stream functions.

#ifndef _PUBLIC_UTIL_SERIAL_UTILS_STREAM_UTIL_H_
#define _PUBLIC_UTIL_SERIAL_UTILS_STREAM_UTIL_H_

#include <istream>
#include <sstream>
#include <unordered_set>

#include "base/common.h"
#include "base/system.h"
#include "util/string/strutil.h"

extern bool gFlag_serialization_debug_parse_error;

// Utility Macro to dump the current stream state.
#define LOG_STREAM_STATE(str)  LOG(INFO) << "Good: " << str.good() << ", Fail: " << str.fail() \
    << ", EOF: " << str.eof() << ", " << "Pos: " << str.tellg()

namespace serial {
namespace util {

// Logs the parsing error.
inline void LogParsingError(istream& in, istream::pos_type pos,
                            const string& prefix = "", string* err = nullptr) {
  in.clear();  // Clear the error state temporarily.
  if (pos < 0) pos = in.tellg() < 10 ?
      istream::pos_type(0) : in.tellg() - 10l;
  in.seekg(pos);
  string c(32, '\0');
  in.read(const_cast<char*>(c.c_str()), 32);
  c.resize(in.gcount());

  stringstream ss;
  ss << "Parsing Error: Failed at pos : " << static_cast<int64_t>(pos) << ". "
     << prefix << " ..." << strutil::EscapeString_C(c) << "...";
  if (gFlag_serialization_debug_parse_error && !SysInfo::Instance().InProduction()) {
    ss << endl;
    PrintStackTraceToStream(ss);
  }

  const string str = ss.str();
  LOG(INFO) << str;
  if (err != nullptr) *err += str + '\n';

  in.setstate(istream::failbit);
}

// Skips all white spaces.
// Currently we assume all characters less than 0x20 as white space.
// In case this needs to change in future, we may want to use std::isspace().
// Note: We optimized this function for production case, where we expect
// almost no extra spaces.
inline void SkipSpaces(istream& in) {
  while (true) {
    char ch = in.peek();
    if (ch > ' ' || in.fail()) break;
    ch = in.get();
  }
}

// Skips all white spaces and returns the next character.
inline char SkipToNextChar(istream& in, bool consume = false) {
  SkipSpaces(in);
  return !in.good() ? 0 : consume ? in.get() : in.peek();
}

// Returns the next token upto the basic escape characters.
// Note that this is a stripped down, dirty way of getting a token.
// For complex tokens, it is better to extract the token while unescaping stream
// at the same time.
// Returns true of the operation was successful and false otherwise.
inline bool ExtractDelimited(istream& in, string* res,
                             const string& delims = " ,\n\r") {
  ASSERT_NOTNULL(res);
  res->reserve(32);  // Optimization.
  while (true) {
    char ch = in.get();
    if (in.fail()) break;
    if (delims.find(ch) != string::npos) {
      in.unget();
      break;
    }
    res->push_back(ch);
  }
  res->shrink_to_fit();
  return !in.fail();
}

// Extracts the token from the stream till the next quote. This also consumes
// the next quote.
inline bool ExtractToNextQuote(istream& in, string* res) {
  ASSERT_NOTNULL(res);
  res->reserve(32);  // Optimization.
  while (true) {
    char ch = in.get();
    if (in.fail()) break;
    if (ch == '"') break;
    res->push_back(ch);
  }
  res->shrink_to_fit();
  return !in.fail();
}

inline string ExtractDelimited(istream& in, const string& delims = " ,\n\r") {
  string str;
  ExtractDelimited(in, &str, delims);
  return str;
}

inline bool ExtractQuotedString(istream& in, string* res,
                                const string& fallback_delimiters = " ,\n\r",
                                bool* was_quoted = nullptr) {
  ASSERT_NOTNULL(res);
  // Skip spaces upto the first valid char.
  util::SkipSpaces(in);
  if (in.fail()) return false;

  bool quoted = true;
  char ch = in.get();
  if (in.fail()) return false;

  if (ch == '"') {
    ExtractToNextQuote(in, res);
  } else if (fallback_delimiters.size()) {
    in.unget();
    ExtractDelimited(in, res, fallback_delimiters);
    quoted = false;
  }

  if (was_quoted != nullptr) *was_quoted = quoted;
  return !in.fail();
}

// Skips a string upto the one of the delimited characters.
// Ignores escpaed delimited characters if required.
inline bool SkipDelimitedString(istream& in, const string& delims = " ,\n\r",
                                bool skip_escaped = false) {
  int skip_next = 0;
  while (true) {
    char ch = in.get();
    if (in.fail()) break;
    if (!skip_next && (delims.find(ch) != string::npos)) {
      in.unget();
      break;
    }
    if (skip_escaped) {
      if (ch == '\\') (++skip_next) %= 2;
      else skip_next = 0;
    }
  }
  return !in.fail();
}

// Skips a *possibly* quoted string upto the one of the delimited characters.
// Ignores escpaed delimited characters if required.
inline void SkipQuotedString(istream& in,
                             const string& fallback_delimiters = "",
                             bool skip_escaped = false) {
  // Skip spaces upto the first valid char.
  util::SkipSpaces(in);
  if (in.fail()) return;

  char ch = in.get();
  if (in.fail()) return;

  if (ch == '"') {
    SkipDelimitedString(in, "\"", true);
    if (!in.good()) return;
    // Read the ending '"'.
    ch = in.get();
  } else if (fallback_delimiters.size()) {
    in.unget();
    SkipDelimitedString(in, fallback_delimiters, skip_escaped);
  }
  return;
}

// Returns true if the next non-white space character is one of the
// characters in the expected list. If consume is set, the character is
// also consumed.
inline char ExpectNext(istream& in, const string& expected,
                       bool consume = true, string* err = nullptr) {
  char ch = util::SkipToNextChar(in, consume);
  if (expected.find(ch) != string::npos) return ch;

  istream::pos_type pos = in.tellg() - 1l;
  stringstream ss;
  ss << "Did not find any of expected chars: " << expected  << " found '" << ch
     << "' instead.";
  LogParsingError(in, pos, ss.str(), err);
  return 0;
}

}  // namespace util
}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_UTILS_STREAM_UTIL_H_
