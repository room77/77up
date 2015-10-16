//
// Copyright 2007 OpTrip, Inc.
//
// Author: Calvin Yang
//
#include "strutil.h"

#include <unistd.h>

namespace strutil {

// check if s1 is a prefix of s2 (case-insensitive)
bool IsPrefix(const char *s1, const char *s2) {
  while ((*s1) != '\0' && (*s2) != '\0') {
    if (toupper(*s1) != toupper(*s2))
      return false;
    s1++;
    s2++;
  }
  return ((*s1) == '\0');
}

// check if a string contains a given word (whole word only, case-insensitive)
bool ContainsWord(const string& haystack, const string& needle) {
  int needle_len = needle.size();
  const char *s = haystack.c_str();
  while ((*s) != '\0') {
    // skip until the beginning of the next word
    while (!IS_ALPHANUMERIC(*s) && (*s) != '\0')
      s++;
    if ((*s) != '\0') {
      if (strncasecmp(s, needle.c_str(), needle_len) == 0 &&
          !IS_ALPHANUMERIC(*(s + needle_len)))
        return true;
      // forward to the end of this word
      while (IS_ALPHANUMERIC(*s))
        s++;
    }
  }
  return false;
}


// indent a string by a number of spaces for each line
string Indent(const string& original, int num_indent) {
  string s;
  bool is_new_line = true;
  for (int i = 0; i < original.size(); i++) {
    if (is_new_line) {
      s += string(num_indent, ' ');
      is_new_line = false;
    }
    char c = original[i];
    s += c;
    if (c == '\n')
      is_new_line = true;
  }
  return s;
}

string TwoDigitHex(unsigned char c) {
  int h1, h2;
  h1 = c >> 4;
  h2 = c & 15;
  string s;
  s += (h1 < 10 ? ('0' + h1) : (h1 - 10 + 'A'));
  s += (h2 < 10 ? ('0' + h2) : (h2 - 10 + 'A'));
  return s;
}

string FourDigitHex(unsigned short c) {
  unsigned char h1 = c >> 8;
  unsigned char h2 = c & 255;
  return TwoDigitHex(h1) + TwoDigitHex(h2);
}

inline string UnicodeEscaped(unsigned short c) {
  // special case: these code points do not seem cross-browser
  if (c >= 0x91 && c <= 0x94) {
    if (c == 0x91)
      c = '`';
    else if (c == 0x92)
      c = '\'';
    else
      c = '"';
  }
  return string("\\u") + FourDigitHex(c);
}

// json-encode a string of 8-bit characters that are not UTF-8 encoded
string JsonEncode8Bit(const string& s) {
  string encoded;
  for (int i = 0; i < s.size(); i++) {
    unsigned char c = s[i];
    encoded += UnicodeEscaped(c);
  }
  return encoded;
}

//
// escape/unescape functions
//
//   -- C-style: escape using backslash
//      (if json_style is true, use \u???? instead of \??? escape)
string EscapeString_C_or_JSON(const string& input, bool json_style) {
  string s = "\"";
  const char *current = input.c_str();
  int input_len = input.size();
  string unencoded;
  unsigned int buf = 0;
  int expect_more = 0;
  for (int i = 0; i < input_len; ++i, ++current) {
    unsigned char c = *current;
    if (json_style && c >= 128) {
      // assume this is multi-byte utf-8 encoding
      //

      unencoded.push_back(c);

      if (c >= 254) {
        expect_more = 0;  // invalid character in utf-8 -- error in input
        s += JsonEncode8Bit(unencoded);
        unencoded.clear();
      }
      else if (c >= 252) {   // 1111110x in binary
        // first byte of 6-byte utf-8 character
        buf = (c & 1);
        expect_more = 5;
      }
      else if (c >= 248) {   // 111110xx in binary
        // first byte of 5-byte utf-8 character
        buf = (c & 3);
        expect_more = 4;
      }
      else if (c >= 240) {   // 11110xxx in binary
        // first byte of 4-byte utf-8 character
        buf = (c & 7);
        expect_more = 3;
      }
      else if (c >= 224) {   // 1110xxxx in binary
        // first byte of 3-byte utf-8 character
        buf = (c & 15);
        expect_more = 2;
      }
      else if (c >= 192) {   // 110xxxxx in binary
        // first byte of 2-byte utf-8 character
        buf = (c & 31);
        expect_more = 1;
      }
      else {
        // subsequent byte of a multi-byte utf-8 character
        if (expect_more > 0) {
          buf = (buf << 6) + (c & 63);
          expect_more--;
          if (expect_more == 0) {
            if (buf <= 65535) {
              s += UnicodeEscaped(buf);
            }
            else {
              s += UnicodeEscaped(buf >> 16);
              s += UnicodeEscaped(buf & 65535);
            }
            // successfully parsed an UTF-8 character
            unencoded.clear();  // not needed anymore
          }
        }
        else {
          // not valid UTF-8
          expect_more = 0;
          s += JsonEncode8Bit(unencoded);
          unencoded.clear();
        }
      }
    }
    else {
      if (expect_more > 0) {
        expect_more = 0;  // error in input string -- not utf-8?
        s += JsonEncode8Bit(unencoded);
        unencoded.clear();
      }

      if (c < 32) {
        switch (c) {
        case '\n': s += "\\n"; break;
        case '\t': s += "\\t"; break;
        case '\r': s += "\\r"; break;
        case '\b': s += "\\b"; break;
        case '\f': s += "\\f"; break;
        default: {
          if (json_style) {
            // output hex representation of c
            s += UnicodeEscaped(c);
            break;
          }
          else {
            // output octal representation of c
            int o1, o2, o3;
            o1 = c >> 6;
            o2 = (c & 63) >> 3;
            o3 = (c & 7);
            s += '\\';
            s += ('0' + o1);
            s += ('0' + o2);
            s += ('0' + o3);
            break;
          }
        }
        }
      }
      else if (c == '"' || c == '\\') {
        s += '\\';
        s += c;
      }
      else
        s += c;
    }
  }
  s += '"';
  return s;
}

//   -- unescape a C-style escaped string
//   (Returns the number of bytes parsed.
//    Parsed string is in "result" if it's not NULL.)
int UnescapeString_C(const char *input, string *result) {
  if (result) result->clear();

  const char *s = SkipSpaces(input);
  if (*s != '"') {
    VLOG(3) << "missing quotation mark for string: " << input;
    return 0;
  }
  s++;

  while (*s != '\0') {
    if (*s == '"') {
      // end of string
      s++;
      break;
    }
    if (*s == '\\') {
      s++;
      switch (*s) {
      case 'n':
        if (result) result->push_back('\n');
        break;
      case 't':
        if (result) result->push_back('\t');
        break;
      case 'r':
        if (result) result->push_back('\r');
        break;
      case 'b':
        if (result) result->push_back('\b');
        break;
      case 'f':
        if (result) result->push_back('\f');
        break;
      case '"':
        if (result) result->push_back('"');
        break;
      case '/':
        if (result) result->push_back('/');
        break;
      case '\\':
        if (result) result->push_back('\\');
        break;
      case 'u': {
        if (IS_HEXDIGIT(*(s + 1)) && IS_HEXDIGIT(*(s + 2)) &&
            IS_HEXDIGIT(*(s + 3)) && IS_HEXDIGIT(*(s + 4))) {
          char h[5];
          strncpy(h, s + 1, 4);
          h[4] = '\0';
          char *end;
          unsigned int d = strtoul(h, &end, 16);
          if (d >= 256) {
            VLOG(3) << "Universal characters are not yet supported: " << input;
            // ignore for now
          }
          else
            if (result)
              result->push_back(static_cast<unsigned char>(d));
          s += 4;
        }
        break;
      }
      default: {
        if (IS_DIGIT(*s) && IS_DIGIT(*(s + 1)) && IS_DIGIT(*(s + 2))) {
          char c = (*s - '0') * 64 + (*(s + 1) - '0') * 8 + (*(s + 2) - '0');
          if (result)
            result->push_back(c);
          s += 2;
        }
        else {
          // unrecognized control character
          VLOG(3) << "Unrecognized control character inside string: " << input;
          // ignore for now
        }
        break;
      }
      }
      s++;
    }
    else {
      if (result)
        result->push_back(*s);
      s++;
    }
  }

  return s - input;  // number of bytes parsed
}

//   -- unescape a CSV string
//   (Returns the number of bytes parsed.
//    Parsed string is in "result" if it's not NULL.)
int UnescapeString_CSV(const char *input, string *result) {
  if (result) result->clear();

  const char *s = SkipSpaces(input);
  if (*s != '"') {
    VLOG(3) << "missing quotation mark for string: " << input;
    return 0;
  }
  ++s;

  while (*s != '\0') {
    if (*s == '"' && *++s != '"') break;
    if (result) result->push_back(*s);
    ++s;
  }

  return s - input;  // number of bytes parsed
}

//   -- CGI-style: escape using %nn
string EscapeString_CGI(const string& input, bool space_to_plus) {
  string s = "";
  for (int i = 0; i < input.size(); i++) {
    unsigned char c = input[i];
    if (IS_DIGIT(c) || IS_LETTER(c))
      s += c;
    else if (c == ' ' && space_to_plus)
      s += '+';
    else {
      // convert c to %nn format (hex)
      unsigned char c1 = c / 16;
      unsigned char c2 = c % 16;
      s += '%';
      s += (c1 > 9 ? ('A' + c1 - 10) : ('0' + c1));
      s += (c2 > 9 ? ('A' + c2 - 10) : ('0' + c2));
    }
  }
  return s;
}

string UnescapeString_CGI(const string& input) {
  string result = "";
  const char *s = input.c_str();

  while (*s != '\0') {
    if (*s == '%') {
      if (IS_HEXDIGIT(*(s + 1)) && IS_HEXDIGIT(*(s + 2))) {
        s++;
        unsigned char c1 = (IS_DIGIT(*s) ?
                            ((*s) - '0') : (toupper(*s) - 'A' + 10));
        s++;
        unsigned char c2 = (IS_DIGIT(*s) ?
                            ((*s) - '0') : (toupper(*s) - 'A' + 10));
        unsigned char c = c1 * 16 + c2;
        result.push_back(c);
      }
      else {
        // unrecognized control character
        VLOG(5) << "Unrecognized control character inside string: " << input;
        // ignore for now
      }
    }
    else if (*s == '+')
      result.push_back(' ');
    else
      result.push_back(*s);
    s++;
  }
  return result;
}


//   -- HTML-style: escape < > & etc.
string EscapeString_HTML(const string& input) {
  string s = "";
  for (int i = 0; i < input.size(); i++) {
    unsigned char c = input[i];
    switch (c) {
    case '<': s += "&lt;"; break;
    case '>': s += "&gt;"; break;
    case '&': s += "&amp;"; break;
    default:  s.push_back(c); break;
    }
  }
  return s;
}

// attempt to replace "original" with "to_replace"
// (if replaced, return a pointer to the last character of the original;
//  if not found, return "begin" itself)
const char *ReplaceEscape(const char *begin, const string& original,
                          const string& to_replace, string *result) {
  if (!strncmp(begin, original.c_str(), original.size())) {
    // match
    (*result) += to_replace;
    return begin + original.size() - 1;
  }
  else {
    // not a match
    return begin;
  }
}

string UnescapeString_HTML(const char *s) {
  string result = "";
  result.reserve(strlen(s)*2);

  while (*s != '\0') {
    if (*s == '&') {
      const char *begin = s;

      s = ReplaceEscape(s, "&lt;", "<", &result);
      s = ReplaceEscape(s, "&gt;", ">", &result);
      s = ReplaceEscape(s, "&amp;", "&", &result);
      s = ReplaceEscape(s, "&nbsp;", " ", &result);
      s = ReplaceEscape(s, "&quot;", "\"", &result);
      s = ReplaceEscape(s, "&apos;", "'", &result);
      s = ReplaceEscape(s, "&reg;", "(R)", &result);
      s = ReplaceEscape(s, "&Eacute;", "E", &result);
      s = ReplaceEscape(s, "&eacute;", "e", &result);
      s = ReplaceEscape(s, "&lsquo;", "`", &result);
      s = ReplaceEscape(s, "&rsquo;", "'", &result);
      s = ReplaceEscape(s, "&acirc;", "a", &result);
      s = ReplaceEscape(s, "&uuml;", "u", &result);
      s = ReplaceEscape(s, "&ndash;", "-", &result);
      s = ReplaceEscape(s, "&euro;", "euro ", &result);

      if (s == begin) {
        // unrecognized control character
        const char *s1 = s + 1;
        if (IS_ALPHANUMERIC(*s1) || (*s1 == '#')) {
          s1++;
          while (IS_ALPHANUMERIC(*s1))
            s1++;
        }
        if (*s1 == ';') {
          VLOG(5) << "Unrecognized control character inside string: " << s;
          // unrecognized escape in the form of &foobar;
          // -- ignore for now
          s = s1;
        }
        else {
          // not in the form of &foobar;
          // ignore for now  -- treat as unescaped '&'
          result.push_back('&');
        }
      }
    }
    else
      result.push_back(*s);
    s++;
  }
  return result;
}

// return base64 encoding of a string
// (if split_line is true, add a newline every 76 output characters,
//  according to MIME specs)
string EncodeString_Base64(const string& input, bool split_line) {
  // 64 characters used for base-64 encoding
  const char *base64chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  string result;
  int num_output_chars = 0;
  int len = input.size();
  for (int i = 0; i < len; i++) {
    unsigned char c1 = input[i];
    unsigned char c2 ='\0';
    unsigned char c3 ='\0';

    // get the next three characters
    int num_fillers = 0;
    if (i < len - 2) {
      c2 = input[++i];
      c3 = input[++i];
      num_fillers = 0;
    }
    else if (i == len - 2) {
      c2 = input[++i];
      num_fillers = 1;
    }
    else if (i == len - 1) {
      num_fillers = 2;
    }

    // add a newline after every 76 spec output characters (MIME spec)
    if (split_line && num_output_chars > 0 && num_output_chars % 76 == 0)
      result.push_back('\n');

    // concatenate the 3 characters into a 24-bit integer
    unsigned int n = (((static_cast<unsigned int>(c1)) << 16) +
                      ((static_cast<unsigned int>(c2)) << 8) +
                      c3);
    // then separate it into four 6-bit numbers
    result.push_back(base64chars[(n >> 18) & 63]);
    result.push_back(base64chars[(n >> 12) & 63]);
    result.push_back((num_fillers == 2) ? '=' : base64chars[(n >> 6) & 63]);
    result.push_back((num_fillers >= 1) ? '=' : base64chars[n & 63]);
    num_output_chars += 4;
  }
  return result;
}

// decode base64-encoded string
string DecodeString_Base64(const string& input) {
  string result;
  const char *begin = input.c_str();
  const char *s = strutil::SkipSpaces(begin);
  while (*s != '\0') {
    // parse the next four characters as base64 chars
    unsigned int n = 0;  // 24-byte integer
    int num_fillers = 0;
    for (int i = 0; i < 4; i++) {
      char next = *(s + i);
      unsigned char c;
      if (next >= 'A' && next <= 'Z')
        c = next - 'A';
      else if (next >= 'a' && next <= 'z')
        c = next - 'a' + 26;
      else if (next >= '0' && next <= '9')
        c = next - '0' + 52;
      else if (next == '+')
        c = 62;
      else if (next == '/')
        c = 63;
      else if (next == '=') {
        // end of string is near
        num_fillers++;
        c = 0;
      }
      else {
        // an error has occurred
        VLOG(3) << "Error near char " << (s + i - begin + 1) << ": " << input;
        return "";
      }
      n = (n << 6) + c;
    }
    s = strutil::SkipSpaces(s + 4);
    if (num_fillers > 0) {
      if (*s != '\0' || num_fillers > 2) {
        // an error has occurred
        VLOG(3) << "Error near char " << (s - begin + 1) << ": " << input;
        return "";
      }
    }
    // now n is a 24-bit integer which needs to be split into 3 bytes
    result.push_back(n >> 16);
    if (num_fillers <= 1)
      result.push_back((n >> 8) & 255);
    if (num_fillers == 0)
      result.push_back(n & 255);
  }
  return result;
}


// make a copy of a string
// Note: caller is responsible for freeing space allocated
char *CopyString(const char *src) {
  char *dest = new char[strlen(src) + 1];
  strcpy(dest, src);
  return dest;
}


// make a copy of a string (up to N chars)
// Note: caller is responsible for freeing space allocated
char *CopyStringLengthN(const char *src, int N) {
  char *dest = new char[N + 1];
  strncpy(dest, src, N);
  dest[N] = '\0';
  return dest;
}

void FixSpecialCapitalization(const string& from, const string& to,
                              int end_index, string& s) {
  int len = from.size();
  ASSERT_EQ(len, to.size());
  int start = end_index - len + 1;
  if (start >= 0) {
    if (start == 0 || !IS_LETTER(s[start - 1])) {
      for (int i = 0; i < len; i++)
        if (tolower(s[start + i]) != tolower(from[i]))
          return;  // not a match
      // update the string in-place
      for (int i = 0; i < len; i++)
        s[start + i] = to[i];
    }
  }
}

string Capitalize(const string& input) {
  string s = input;
  for (int i = 0; i < s.size(); i++) {
    char prev = (i == 0 ? ' ' : s[i - 1]);
    if (IS_LETTER(s[i]) &&
        !IS_LETTER(prev) && !IS_DIGIT(prev) && prev != '\'')
      s[i] = toupper(s[i]);
    else
      s[i] = tolower(s[i]);

    if (i == s.size() - 1 || (IS_LETTER(s[i]) && !IS_LETTER(s[i + 1]))) {
      FixSpecialCapitalization("us", "US", i, s);
      FixSpecialCapitalization("usa", "USA", i, s);
      FixSpecialCapitalization("de", "de", i, s);
      FixSpecialCapitalization("del", "del", i, s);
      FixSpecialCapitalization("and", "and", i, s);
      FixSpecialCapitalization("at", "at", i, s);
      FixSpecialCapitalization("of", "of", i, s);
      FixSpecialCapitalization("the", "the", i, s);
      FixSpecialCapitalization("to", "to", i, s);
      FixSpecialCapitalization("la", "la", i, s);
      FixSpecialCapitalization("nw", "NW", i, s);
      FixSpecialCapitalization("ne", "NE", i, s);
      FixSpecialCapitalization("sw", "SW", i, s);
      FixSpecialCapitalization("se", "SE", i, s);
      FixSpecialCapitalization("id", "ID", i, s);
      FixSpecialCapitalization("aaa", "AAA", i, s);
      FixSpecialCapitalization("caa", "CAA", i, s);
      FixSpecialCapitalization("aarp", "AARP", i, s);
      FixSpecialCapitalization("ada", "ADA", i, s);
      FixSpecialCapitalization("usd", "USD", i, s);
      FixSpecialCapitalization("cad", "CAD", i, s);
      FixSpecialCapitalization("eur", "EUR", i, s);
      FixSpecialCapitalization("jpy", "JPY", i, s);
    }
  }
  VLOG(4) << "Fixing capitalization: " << input << " -> " << s;
  return s;
}

void AllUpperToAllLower(string& input) {
  if (!ContainsLowerCase(input)) {
    bool beginning = true;
    for (int i = 0; i < input.size(); i++) {
      char c = input[i];
      if (c == '.')
        beginning = true;
      else if (c >= 'A' && c <= 'Z') {
        if (beginning)
          beginning = false;  // beginning of a sentence remains capitalized
        else
          input[i] = tolower(c);  // all subsequent letters are lower-case
      }
    }
  }
}

// check if two strings have the same alphanumeric content
// (case-insensitive, excluding non-alphanumeric characters)
bool SameName(const string& str1, const string& str2) {
  const char *s1 = str1.c_str();
  const char *s2 = str2.c_str();
  while (1) {
    // skip until the next alphanumeric character
    while (*s1 != '\0' && !IS_DIGIT(*s1) && !IS_LETTER(*s1))
      s1++;
    while (*s2 != '\0' && !IS_DIGIT(*s2) && !IS_LETTER(*s2))
      s2++;
    if (toupper(*s1) != toupper(*s2))
      return false;

    if ((*s1) == '\0') {
      ASSERT((*s2) == '\0');
      return true;
    }
    ASSERT((*s2) != '\0');

    s1++; s2++;
  }
  // control never reaches here
}


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
                           int result_size_limit, string *result) {
  vector<char> buf(bytes_to_read);
  int read_count = read(ear, &buf[0], bytes_to_read);
  if (read_count <= 0)
    return -1;  // no data is available

  return AppendDataToString(&buf[0], read_count, result_size_limit, result);
}


// Append data to a size-limited string
// (if result_size_limit < 0, then there is no size limit)
// Return value is the actual number of bytes appended
int AppendDataToString(const char *data, int len,
                       int result_size_limit, string *result) {
  if (len <= 0) return 0;

  if (result_size_limit < 0) {
    // No size limit.  Just append the string and return.
    result->append(string(data, len));
    return len;
  }
  else {
    // There is a size limit.  Check how much free space we have
    int free_space = result_size_limit - result->size();

    if (len <= free_space) {
      // we have enough space -- append the entire string
      result->append(string(data, len));
      return len;
    }
    else {
      // we don't have enough space -- append as much as we can
      if (free_space > 0) {
        result->append(string(data, free_space));
      }
      return free_space;
    }
  }
}

// return characters inside a string surrounding a given location and
// pinpoint that location  (as debug info)
// -- for example:
//
//   this is a string with an error.
//                            ^
//
string PrintNeighborhood(const char *string_start, const char *pinpoint,
                         bool two_line_format) {
  stringstream ss;
  ASSERT(string_start != NULL && pinpoint != NULL && pinpoint >= string_start);
  const int neighborhood = 20;

  const char *start = (pinpoint > (string_start + neighborhood) ?
                       (pinpoint - neighborhood) : string_start);
  if (start > string_start)
    ss << "... ";

  const char *s;
  for (s = start; s < start + neighborhood * 2; s++) {
    if (s == pinpoint)
      ss << '^';
    if (*s == '\0') break;
    char c = *s;
    ss << (c > ' ' ? c : ' ');
    if (s == pinpoint)
      ss << '^';
  }
  if (*s != '\0')
    ss << " ...";

  if (two_line_format) {
    ss << "\n";

    if (start > string_start)
      ss << "    ";
    for (s = start; s < pinpoint; s++)
      ss << ' ';
    ss << "^\n";
  }

  return ss.str();
}


// replace all occurrence of a substring to another one
string ReplaceAll(const string& src, const string& from, const string& to) {
  const char *from_s = from.c_str();
  int len = from.size();
  string output;
  const char *s = src.c_str();
  while ((*s) != '\0') {
    if (strncmp(s, from_s, len) == 0) {
      output += to;
      s += len;
    }
    else {
      output += *s;
      s++;
    }
  }
  return output;
}

// debug function: print a string's raw content (ascii code and characters)
void PrintRaw(const string& s) {
  int len = s.size();
  for (int i = 0; i < len; i++) {
    char c = s[i];
    cout << setiosflags(ios::right) << setw(3) << static_cast<int>(c)
         << " " << setw(1) << (c >= 32 ? c : ' ') << "   ";
  }
  cout << endl;
}

// returns a human-readable error message caused by the previous system call
string LastSystemError() {
  char buf[1024];
  strerror_r(errno, buf, 1024);  // thread-safe
  return buf;
}

// Filters out non-letter characters; when applicable, converts to
// lower-case letters
const string ExtractText(const string& s, const char dummy) {
  const static char DIFF = 'a' - 'A';
  string ret;
  ret.reserve(s.size());
  bool was_dummy = true;
  for (unsigned char c : s) {
    if ((c >= 'a' && c <= 'z') || c >= 128) {
      ret += c;
      was_dummy = false;
    } else if (c >= 'A' && c <= 'Z') {
      ret += (c + DIFF);
      was_dummy = false;
    } else {
      // Treat all else as dummy
      if (!was_dummy) ret += dummy; // Avoid repeated dummy
      was_dummy = true;
    }
  }

  if (was_dummy && !ret.empty()) ret.pop_back();
  return ret;
}

string ReplaceJunkMultiByteChars(const char *s) {
  string result = "";
  result.reserve(strlen(s));
  for (; *s != '\0'; ++s) {
    const char* begin = s;
    if (*s == '\xc3') {
      s = ReplaceEscape(s, "â€œ", "“", &result);
      s = ReplaceEscape(s, "â€\x9d", "”", &result);
      s = ReplaceEscape(s, "â€™", "’", &result);
      s = ReplaceEscape(s, "â€˜", "‘", &result);
      s = ReplaceEscape(s, "â€”", "–", &result);
      s = ReplaceEscape(s, "â€\"", "-", &result);
      s = ReplaceEscape(s, "â€“", "—", &result);
      s = ReplaceEscape(s, "â€¢", "-", &result);
      s = ReplaceEscape(s, "â€¦", "…", &result);
      s = ReplaceEscape(s, "Ã©", "é", &result);
      s = ReplaceEscape(s, "Ã¨", "è", &result);
      s = ReplaceEscape(s, "Ã¢", "ã", &result);
      s = ReplaceEscape(s, "Ã\"", "ä", &result);
      s = ReplaceEscape(s, "Ã¼", "ü", &result);
      s = ReplaceEscape(s, "Ã,Ã", "'", &result);
      s = ReplaceEscape(s, "Â´", "'", &result);
      s = ReplaceEscape(s, "Ä«", "ī", &result);
      s = ReplaceEscape(s, "é˜´", "阴", &result);
      s = ReplaceEscape(s, "é™°", "陰", &result);
      s = ReplaceEscape(s, "é˜³", "阳", &result);
      s = ReplaceEscape(s, "é™½", "陽", &result);
    }
    if (s == begin) result.push_back(*s);
  }
  return result;
}

///////////////////////////////////////
// Moved from parserutil library:
//

// break up a string into segments (using the given separator or \n, \r, etc.)
int BreakUpString(const string& str, char separator, vector<string> *v) {
  const char *s = str.c_str();
  const char *end = s + str.size() - 1;
  v->clear();

  const char *next = strchr(s, separator);
  while (next != NULL) {
    v->push_back(GetTrimmedString(s, next - 1));
    s = next + 1;
    next = strchr(s, separator);
  }
  v->push_back(GetTrimmedString(s, end));

  // special case: if input is empty, then do not put anything into result
  if (v->size() == 1 && (*v)[0].empty())
    v->clear();

  return v->size();
}

// normalize a "name" string by replacing adjacent non-alphanumeric characters
// by a single space character
string NormalizeName(const string& str) {
  string result;
  const char *s = strutil::SkipNonAlphaNumeric(str.c_str());
  while ((*s) != '\0') {
    if (IS_ALPHANUMERIC_INTL(*s)) {
      result.push_back(*s);
      s++;
    } else {
      s = strutil::SkipNonAlphaNumeric(s);
      if ((*s) != '\0')
        result.push_back(' ');
    }
  }
  return result;
}

// normalize a "name" string into a vector of "words" (by removing
// non-alphanumeric characters)
int NormalizeNameIntoWords(const string& str, vector<string> *v) {
  const char *s = strutil::SkipNonAlphaNumeric(str.c_str());
  while ((*s) != '\0') {
    const char *start = s;
    s = strutil::SkipAlphaNumeric(s);
    v->push_back(string(start, s - start));
    s = strutil::SkipNonAlphaNumeric(s);
  }
  return v->size();
}

// parse as a "trimmed" string (removing leading/trailing spaces)
string GetTrimmedString(const char *start, const char *end) {
  ASSERT(start != NULL && end != NULL);

  while (start <= end && (*start) <= ' ')
    start++;
  while (start <= end && (*end) <= ' ')
    end--;

  return (start <= end ? string(start, end - start + 1) : "");
}

// remove leading and trailing empty lines
void TrimLines(string& s) {
  const char *start = s.c_str();
  const char *end = start + s.size() - 1;
  const char *s1 = start;

  // skip leading spaces and newlines
  while (s1 <= end && (*s1) <= ' ')
    s1++;
  // preserve leading spaces of the first line
  while (s1 > start && (*(s1 - 1)) == ' ')
    s1--;
  // skip trailing spaces
  bool trailing_newline = false;
  while (s1 <= end && (*end) <= ' ') {
    if ((*end) == '\n') trailing_newline = true;
    end--;
  }

  string newstring = (s1 <= end ? string(s1, end - s1 + 1) : "");
  s = (trailing_newline ? (newstring + "\r\n") : newstring);
}

string RemoveLastWord(const string& s) {
  int i = s.size() - 1;

  // skip trailing spaces
  while (i >= 0) {
    if (s[i] <= ' ') i--;
    else break;
  }

  // then skip non-space characters
  while (i >= 0) {
    if (s[i] > ' ') i--;
    else break;
  }

  // then skip spaces
  while (i >= 0) {
    if (s[i] <= ' ') i--;
    else break;
  }

  return s.substr(0, i + 1);
}

}  // end of namespace

