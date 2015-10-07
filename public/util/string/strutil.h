//
// Copyright 2007 OpTrip, Inc.
//
// Author: Calvin Yang
//
//
// some string-related utilities
//

#ifndef _PUBLIC_UTIL_STRING_STRUTIL_H_
#define _PUBLIC_UTIL_STRING_STRUTIL_H_

#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

#include <cmath>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <functional>
#include <type_traits>
#include <map>
#include <string>
#include <vector>

#include "base/defs.h"
#include "base/logging.h"

#define IS_DIGIT(s) ((s) >= '0' && (s) <= '9')
#define IS_LETTER(s) (((s) >= 'A' && (s) <= 'Z') || ((s) >= 'a' && (s) <= 'z'))
#define IS_HEXDIGIT(s) (((s) >= '0' && (s) <= '9') || \
                      ((s) >= 'A' && (s) <= 'F') || ((s) >= 'a' && (s) <= 'f'))
#define IS_ALPHANUMERIC(s) (IS_DIGIT(s) || IS_LETTER(s))
#define IS_ALPHANUMERIC_INTL(s) (IS_DIGIT(s) || IS_LETTER(s) || \
                                 (((s) & 128) != 0))


// a collection of string processing functions

namespace strutil {

template<typename T>
typename std::enable_if<!std::is_same<T, string>::value, T>::type FromString(
    string const& x, const T& default_value = T()) {
  T val = T();
  istringstream strm(x);
  strm >> val;
  return strm.fail() ? default_value : val;
}

template<typename T>
typename std::enable_if<std::is_same<T, string>::value, T>::type FromString(
    string const& x, const T& default_value = T()) {
  return x.empty() ? default_value : x;
}

// NOTE returns nan when if allow_nan == true and (x == "nan" or "NaN", ...)
inline double ParseDouble(string const& x, bool allow_nan = false) {
  double ret = strtod(x.c_str(), nullptr);
  if (!allow_nan && std::isnan(ret)) ret = 0;
  return ret;
}

inline int ParseInt(string const& x) {
  return strtol(x.c_str(), nullptr, 10);
}

// skip leading spaces of a string
inline const char *SkipSpaces(const char *s) {
  while ((*s) <= ' ' && (*s) != '\0') s++;
  return s;
}
inline char *SkipSpaces(char *s) {
  while ((*s) <= ' ' && (*s) != '\0') s++;
  return s;
}

// skip non-spaces of a string
inline const char *SkipNonSpaces(const char *s) {
  while ((*s) > ' ') s++;
  return s;
}
inline char *SkipNonSpaces(char *s) {
  while ((*s) > ' ') s++;
  return s;
}

// trims the string in place
inline void Trim(string *str) {
  string::size_type pos = str->find_last_not_of(' ');
  if (pos == string::npos) {
    str->clear();
  } else {
    str->erase(pos + 1);
    pos = str->find_first_not_of(' ');
    if (pos != string::npos) str->erase(0, pos);
  }
}

// trims the string in place and returns a reference to
// the same trimmed string for convenience
inline string Trim(const string& full_str) {
  string str = full_str;
  Trim(&str);
  return str;
}

// strip html tags. only use for very basic
// cases in which looking for the '<' and '>'
// will be sufficient. for instance, will fail
// when these characters are inside a tag, e.g.
// <a href='http://www.com?q=<foo>'/>
inline string StripTags(const string& html_str) {
  string str = html_str;
  bool found_tag_end = false;
  string::iterator tag_end_it, it = str.end() - 1;
  while(it >= str.begin()) {
    if (!found_tag_end && *it == '>') {
      tag_end_it = it + 1;
      found_tag_end = true;
    }
    else if (found_tag_end && *it == '<') {
      str.erase(it, tag_end_it);
      found_tag_end = false;
    }
    it--;
  }
  return str;
}

// remove trailing spaces of a string (in-place)
inline void RemoveTrailingSpaces(char *s) {
  ASSERT(s != NULL);
  char *tail = s + strlen(s) - 1;
  while (tail >= s && (*tail) <= ' ') {
    (*tail) = '\0';
    tail--;
  }
}

// skip alphanumeric characters
inline const char *SkipAlphaNumeric(const char *s) {
  while (IS_ALPHANUMERIC_INTL(*s))
    s++;
  return s;
}

// skip non-alphanumeric characters
inline const char *SkipNonAlphaNumeric(const char *s) {
  while ((*s) != '\0' && !IS_ALPHANUMERIC_INTL(*s))
    s++;
  return s;
}

inline bool IsNumeric(const string& str) {
  return str.find_first_not_of("0123456789") == string::npos;
}

// check if s1 is a prefix of s2 (case-insensitive)
bool IsPrefix(const char *s1, const char *s2);
inline bool IsPrefix(const char *s1, const string& s2) {
  return IsPrefix(s1, s2.c_str());
}
inline bool IsPrefix(const string& s1, const char *s2) {
  return IsPrefix(s1.c_str(), s2);
}
inline bool IsPrefix(const string& s1, const string& s2) {
  return IsPrefix(s1.c_str(), s2.c_str());
}

// check if a string contains a given word (whole word only, case-insensitive)
bool ContainsWord(const string& haystack, const string& needle);

// construct an error message that reports a message string and an offset
inline string ErrorMsg(const string& message, int offset) {
  stringstream ss;
  ss << "Error at char " << offset << ": " << message;
  return ss.str();
}

string Indent(const string& original, int num_indent);

//
// escape/unescape functions
//
// We have functions to support different styles of escape/unescape:
// operations:
//
// -- C-style:  "abc\n\"\001"
// -- CGI-style: a+b%2A%3C
// -- HTML-style: &nbsp;&lt;abc&gt;

//   -- C-style or JSON-style: escape using backslash
string EscapeString_C_or_JSON(const string& input, bool json_style);
inline string EscapeString_C(const string& input) {
  return EscapeString_C_or_JSON(input, false);
}
inline string EscapeString_JSON(const string& input) {
  return EscapeString_C_or_JSON(input, true);
}

//   -- unescape a C-style (or JSON-style) escaped string
//   (Returns the number of bytes parsed.
//    Parsed string is in "result" if it's not NULL.)
int UnescapeString_C(const char *input, string *result);
inline int UnescapeString_C(const string& input, string *result) {
  return UnescapeString_C(input.c_str(), result);
}
int UnescapeString_CSV(const char *input, string *result);
inline int UnescapeString_CSV(const string& input, string *result) {
  return UnescapeString_CSV(input.c_str(), result);
}
inline int UnescapeString_JSON(const string& input, string *result) {
  return UnescapeString_C(input.c_str(), result);
}
inline int UnescapeString_JSON(const char *input, string *result) {
  return UnescapeString_C(input, result);
}

//   -- CGI-style: escape using %nn
string EscapeString_CGI(const string& input, bool space_to_plus = true);
string UnescapeString_CGI(const string& input);
//   -- HTML-style: escape < > & etc.
string EscapeString_HTML(const string& input);
string UnescapeString_HTML(const char *input);
inline string UnescapeString_HTML(const string& input) {
  return UnescapeString_HTML(input.c_str());
}

// return base64 encoding of a string
// (if split_line is true, add a newline every 76 output characters,
//  according to MIME specs)
string EncodeString_Base64(const string& input, bool split_lines = false);
// decode base64-encoded string
string DecodeString_Base64(const string& input);

// change all characters to upper case
inline string ToUpper(const string& input) {
  string s = input;
  for (int i = 0; i < s.size(); i++)
    s[i] = toupper(s[i]);
  return s;
}
// change all characters to lower case
inline string ToLower(const string& input) {
  string s = input;
  for (int i = 0; i < s.size(); i++)
    s[i] = tolower(s[i]);
  return s;
}

inline bool ContainsLowerCase(const string& input) {
  for (int i = 0; i < input.size(); i++) {
    char c = input[i];
    if (c >= 'a' && c <= 'z') return true;
  }
  return false;
}

// this should be faster than ToLower(s1) == ToLower(s2)
inline bool EqualIgnoreCase(const string& s1, const string& s2) {
  if (s1.size() != s2.size()) return false;
  for (int i = 0; i < s1.size(); ++i) {
    if (tolower(s1[i]) != tolower(s2[i])) return false;
  }
  return true;
}

// capitalize the first letter of each word
string Capitalize(const string& input);

inline void FixCapitalization(string& input) {
  if (!input.empty() && !ContainsLowerCase(input))
    input = Capitalize(input);
}

void AllUpperToAllLower(string& input);

// convert a printable value (such as int) to string
template<class T>
inline string ToString(const T& input) {
  stringstream ss;
  ss << input;
  return ss.str();
}

// Concatenates the elements in the range defined by the iterators.
template<typename ForwardIt>
const string Join(ForwardIt begin, ForwardIt end,
                  const string& separator = ", ") {
  stringstream strm;
  bool first = true;
  for (ForwardIt i = begin; i != end; ++i) {
    if (!first) strm << separator;
    strm << *i;
    first = false;
  }
  return strm.str();
}

// Example: Join(set<int> { 1, 2, 3 }, ":") == "1:2:3"
template<typename T = vector<string>>
const string Join(const T& list, const string& separator = ", ") {
  return Join(list.begin(), list.end(), separator);
}

// Concatenates the strings in the range defined by the iterators.
template<typename ForwardIt>
const string JoinString(ForwardIt begin, ForwardIt end,
                        const string& separator = ", ",
                        bool allow_empty = false) {
  string res;
  bool first = true;
  for (ForwardIt i = begin; i != end; ++i) {
    const string& e = *i;
    if (!allow_empty && e.empty()) continue;
    if (!first) res += separator;
    res += e;
    first = false;
  }
  return res;
}

// Example: JoinString(set<string> { "c", "b", "a" }, ":")
//          == "a:b:c"
template<typename T = vector<string>>
const string JoinString(const T& list, const string& separator = ", ",
                        bool allow_empty = false) {
  return JoinString(list.begin(), list.end(), separator, allow_empty);
}

// Joins two strings using the given operator.
inline const string JoinTwoStrings(const string& left, const string& right,
                                   const string& separator = ", ",
                                   bool allow_empty = false) {
  vector<string> list = {left, right};
  return JoinString(list, separator, allow_empty);
}

// make a copy of a string
// Note: caller is responsible for freeing space allocated
char *CopyString(const char *src);

// make a copy of a string (up to N chars)
// Note: caller is responsible for freeing space allocated
char *CopyStringLengthN(const char *src, int N);

// check if two strings have the same alphanumeric content
// (case-insensitive, excluding non-alphanumeric characters)
bool SameName(const string& str1, const string& str2);

// returns a human-readable error message caused by the previous system call
string LastSystemError();

// read more data from a network socket and append to a size-limited string
// (if result_size_limit < 0, then there is no size limit)
//
// Note: this function will attempt to read bytes_to_read bytes into a buffer
//       first, regardless of result size limit.
//
// Return value:
//   -1:  no data is available for reading (maybe connection closed?)
//   0:   some data is available, but output string is full
//   >0:  actual number of bytes appended
//
int ReadMoreDataFromSocket(int ear, int bytes_to_read,
                                  int result_size_limit, string *result);
// append data to a size-limited string
// (if result_size_limit < 0, then there is no size limit)
// Return value is the actual number of bytes appended
int AppendDataToString(const char *data, int len,
                              int result_size_limit, string *result);

// replace all occurrence of a substring to another one
string ReplaceAll(const string& src, const string& from, const string& to);

// debug function: print a string's raw content (ascii code and characters)
void PrintRaw(const string& s);

// return characters inside a string surrounding a given location and
// pinpoint that location  (as debug info)
// -- for example:
//
//   this is a string with an error.
//                            ^
//
string PrintNeighborhood(const char *string_start, const char *pinpoint,
                         bool two_line_format = true);
inline string PrintNeighborhood(const string& s, int offset,
                                bool two_line_format = true) {
  const char *string_start = s.c_str();
  if (offset > s.size())
    offset = s.size();
  const char *pinpoint = string_start + offset;
  return PrintNeighborhood(string_start, pinpoint, two_line_format);
}

// Returns the edit distance between two strings. It is templated to be able to
// support string and wider char strings (utf32 etc.).
template<typename S>
int EditDistance(const S& s1, const S& s2) {
  const int l1 = s1.size();
  const int l2 = s2.size();
  vector<vector<int> > d(l1 + 1, vector<int>(l2 + 1));

  d[0][0] = 0;
  for(int i = 1; i <= l1; ++i) d[i][0] = i;
  for(int i = 1; i <= l2; ++i) d[0][i] = i;

  for(int i = 1; i <= l1; ++i)
    for(int j = 1; j <= l2; ++j)
      d[i][j] = std::min(std::min(d[i - 1][j] + 1,d[i][j - 1] + 1),
                         d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
  return d[l1][l2];
}

struct NoopTokenNormalizer {
  void operator()(string&) {}
};

// Tokenization utilities.

// This version should work for most non-associative STL containers with a
// value_type string. Examples vector<string>, set<string>. If a vector-like
// container is used, you will get tokens in order of appearance. For a set,
// tokens are deduped and sorted in alphabetical order as expected.
// An optional normalization function can be supplied that can be used to
// modify each token in place (e.g. synonym / abbreviation mapping).
template<class Container>
inline void Tokenize(const string& s, Container* tokens,
    const function<void(string&)>& normalize = NoopTokenNormalizer()) {
  stringstream strm(s);
  for (;;) {
    string token;
    strm >> token;
    if (strm.fail()) break;
    normalize(token);
    tokens->insert(tokens->end(), token);
  }
}

// Specialization for map<string, int> where we count the occurrence of unique
// tokens.
template<>
inline void Tokenize(const string& s, map<string, int>* tokens,
    const function<void(string&)>& normalize) {
  stringstream strm(s);
  for (;;) {
    string token;
    strm >> token;
    if (strm.fail()) break;
    normalize(token);
    ++(*tokens)[token];
  }
}

// Same as above but uses Return Value Optimization (RVO) for convenience.
// It is as efficient as the above and more readable under certain conditions.
// See http://en.wikipedia.org/wiki/Return_value_optimization
//
// Typical usage:
//
// vector<string>   tokens = Tokenize<vector<string> >   (some_string);
// set<string>      tokens = Tokenize<set<string> >      (some_string);
// map<string, int> tokens = Tokenize<map<string, int> > (some_string);
//
template<class Container>
inline Container Tokenize(const string& s,
    const function<void(string&)>& normalize = NoopTokenNormalizer()) {
  Container tokens;
  Tokenize(s, &tokens, normalize);
  return tokens;
}

// tokenize a string into the set of constituent types
// e.g. to tokenize "4 5 6", use:
// vector<int> tokens;
// TokenizeType<int> ("4 5 6", &tokens);
template<typename T>
inline void TokenizeType(const string& s, vector<T> *tokens) {
  stringstream strm(s);
  for (;;) {
    T token;
    strm >> token;
    if (strm.fail()) break;
    tokens->push_back(token);
  }
}

template<typename T>
inline vector<T> TokenizeType(const string& s) {
  vector<T> tokens;
  TokenizeType(s, &tokens);
  return tokens;
}

inline const pair<string, string> ParseStringPair(
    const string& line, const char delimiter = ' ') {
  size_t pos = line.find_first_of(delimiter);
  if (pos == string::npos) return make_pair(line, string(""));

  return make_pair(line.substr(0, pos), line.substr(pos + 1));
}

// General purpose string split function that takes any number of delimiters
// skips consecutive delimiters
inline void SplitSkip(const string& str, vector<string>& tokens, const string& delimiters=",") {
  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);

  // Find first non-delimiter.
  string::size_type pos = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));

    // Skip delimiters
    lastPos = str.find_first_not_of(delimiters, pos);

    // Find next non-delimiter.
    pos = str.find_first_of(delimiters, lastPos);
  }
}

// General purpose string split function that takes any number of delimiters
// outputs empty string between consecutive delimiters
inline void Split(const string& str, vector<string>& tokens, const string& delimiters=",") {
  string::size_type pos, lastPos = 0;
  do {
    pos = str.find_first_of(delimiters, lastPos);
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    lastPos = pos + 1;
  } while (pos != string::npos);
}

// rvo version for convenience
inline const vector<string> Split(const string& str,
                                  const string& delimiters = ",") {
  vector<string> tokens;
  Split(str, tokens, delimiters);
  return tokens;
}

const string ExtractText(const string& s, const char dummy = ' ');

// Replaces junk multibyte characters caused by incorrect conversion between
// unicode and non-unicode charset. e.g. characters like â€œ, â€™, etc.
string ReplaceJunkMultiByteChars(const char *s);
inline string ReplaceJunkMultiByteChars(const string& input) {
  return ReplaceJunkMultiByteChars(input.c_str());
}

// Converts a string to its hex values.
inline string ToHex(const string& str) {
  stringstream ss;
  for (const unsigned char c : str) {
    unsigned short a = c;
    ss << "\\x" << hex << setfill('0') << setw(2) << uppercase << a;
  }
  return ss.str();
}

///////////////////////////////////////
// Moved from parserutil library:
//

// parse as an integer in a fixed-width field
// return the result and set "success" parameter to indicate status
template<class T>
T GetFixedWidthInteger(const char *input, int length, bool *success) {
  const char *p = input;
  bool started = false;
  bool seen_digit = false;
  bool negative = false;
  T r = 0;
  for (int i = 0; i < length; i++) {
    if (*p > ' ') {
      if (*p == '-' && !started)
        negative = true;
      else if (*p == '+' && !started)
        negative = false;
      else if (IS_DIGIT(*p)) {
        seen_digit = true;
        r = r * 10 + (*p - '0');
      }
      else
        break;
      started = true;
    }

    p++;
  }
  if (seen_digit) {
    // successful
    if (success)
      *success = true;
    return (negative ? -r : r);
  }
  else {
    // failed to find an integer
    if (success)
      *success = false;
    return 0;
  }
}

inline int GetFixedWidthInt(const char *input, int length,
                            bool *success) {
  return GetFixedWidthInteger<int>(input, length, success);
}
inline int GetFixedWidthShort(const char *input, int length,
                              bool *success) {
  return GetFixedWidthInteger<short>(input, length, success);
}
inline int GetFixedWidthLongLong(const char *input, int length,
                                 bool *success) {
  return GetFixedWidthInteger<long long>(input, length, success);
}

// parse as a fixed-width string, without removing leading/trailing spaces
inline string GetString(const char *start, int length) {
  ASSERT(start != NULL);
  return string(start, length);
}

// parse as a "trimmed" string (removing leading/trailing spaces)
string GetTrimmedString(const char *start, const char *end);

inline string GetTrimmedString(const char *start, int len) {
  return GetTrimmedString(start, start + len - 1);
}
inline string GetTrimmedString(const string& s) {
  return GetTrimmedString(s.c_str(), s.size());
}
inline string GetTrimmedString(const char *s) {
  return GetTrimmedString(s, strlen(s));
}

// remove leading and trailing empty lines
void TrimLines(string& s);

// remove all spaces from a string (used for prefix index)
inline string RemoveAllSpaces(const string& input) {
  string s;
  for (int i = 0; i < input.size(); i++)
    if (input[i] > ' ')
      s += input[i];
  return s;
}

// return alphanumeric content from a string (used for prefix index)
inline string AlphaNumericContent(const string& input) {
  string s;
  for (int i = 0; i < input.size(); i++) {
    char c = input[i];
    if (IS_ALPHANUMERIC_INTL(c))
      s += c;
  }
  return s;
}

// check if a string contains any non-space content
inline bool IsEmpty(const string& input) {
  for (int i = 0; i < input.size(); i++)
    if (input[i] > ' ')
      return false;
  return true;
}

string RemoveLastWord(const string& s);

// break up a string into segments
// (using the given separator or \n, \r, etc.)
int BreakUpString(const string& str, char separator,
                  vector<string> *v);

// Splits the string into n + 1 segments. e.g. n=1 means the string is split
// into 2 segments.
// e.g. str="ab:c:def:ghi", separators = ":", n = 2 -> {"ab", "c", "def:ghi"}
template <typename Container=vector<string> >
int SplitStringNParts(const string& str, const string& separators, Container *v,
                      int n = std::numeric_limits<int>::max(),
                      bool allow_empty = false) {
  size_t start = 0;
  while (start < str.size() && n > 0) {
    size_t next = str.find_first_of(separators, start);
    if (!allow_empty && next == start) {
      ++start;
      continue;
    }

    v->insert(v->end(), str.substr(start, next - start));
    start = (next == string::npos) ? str.size() : next + 1;
    --n;
  }

  // Insert the last segment.
  if (start < str.size()) v->insert(v->end(), str.substr(start));

  return v->size();
}

// Reverse splits the string into n + 1 segments (iterating from the end).
// e.g. n=1 means the string is split into 2 segments.
// e.g. str="ab:c:def:ghi", separators = ":", n = 2 -> {"ab:c", "def", "ghi"}
template <typename Container=vector<string>>
int RSplitStringNParts(const string& str, const string& separators, Container *v,
                       int n = std::numeric_limits<int>::max(), bool allow_empty = false) {
  int start = static_cast<int>(str.size()) - 1;
  while (start >= 0 && n > 0) {
    size_t next = str.find_last_of(separators, start);
    if (!allow_empty && next == start) {
      --start;
      continue;
    }
    v->insert(v->begin(), str.substr(next + 1, start - next));
    start = next - 1;
    --n;
  }
  // Insert the last segment.
  if (start > -1) v->insert(v->begin(), str.substr(0, start + 1));

  return v->size();
}

// break up a string into segments (using any of the the given separators)
template <typename Container=vector<string> >
int SplitString(const string& str, const string& separators, Container *v,
                bool allow_empty = false) {
  return SplitStringNParts(str, separators, v, std::numeric_limits<int>::max(),
                           allow_empty);
}

// normalize a "name" string by removing punctuations and merging
// adjacent space characters (including \n, \r, etc.) into one
string NormalizeName(const string& str);
int NormalizeNameIntoWords(const string& str, vector<string> *v);

}  // namespace StrUtil

#endif  // _PUBLIC_UTIL_STRING_STRUTIL_H_
