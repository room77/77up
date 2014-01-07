// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/utils/json_util.h"

#include "util/serial/utils/stream_util.h"

namespace serial {
namespace util {

// Skip ahead in the stream when the field is unknown.
void JSONSkipAheadField(istream& in) {
  int level = 0;
  do {
    char ch = SkipToNextChar(in);
    if (!in.good()) break;
    switch (ch) {
      case '[' : case '{' : {
        SkipToNextChar(in, true);
        ++level;
        break;
      }
      case ']' : case '}' : {
        SkipToNextChar(in, true);
        --level;
        break;
      }
      // These two cases are just for optimization.
      case ':' : case ',' : SkipToNextChar(in, true); break;
      default: {
        static const string delims = " \n,:[{}]";
        SkipQuotedString(in, delims, false);
        break;
      }
    }
  } while (in.good() && level != 0);
}

bool JSONSkipAheadAndReturnField(istream& in, string* res) {
  istream::pos_type orig_pos = in.tellg();
  if (orig_pos == -1) return false;

  JSONSkipAheadField(in);
  istream::pos_type new_pos = in.tellg();

  // Check if the if new_pos is invalid.
  if (new_pos == -1) {
    // If this is not EOF, return false.
    if (!in.eof()) return false;

    // In case the stream is at EOF, this probably was an unescaped string.
    // See if we can extract the string back.
    // Clear error state and set the stream to the last element.
    // Currently, we don't worry about resetting the EOF but back. Whoever accesses it after this
    // will fail anyways.
    in.clear();
    in.seekg(0, in.end);
    new_pos = in.tellg();

    // If this still fails, nothing left to do but return.
    if (new_pos == -1) return false;
  }

  int64_t length = new_pos - orig_pos;
  res->resize(length, 0);

  // Start again from the original position and copy the string.
  in.seekg(orig_pos);
  in.read(const_cast<char*>(res->c_str()), length);

  // Check if read failed. Clear the string in that case.
  if (in.fail()) {
    res->clear();
    return false;
  }
  return true;
}

}  // namespace util
}  // namespace serial
