// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_UTIL_SERIAL_UTILS_JSON_UTIL_H_
#define _PUBLIC_UTIL_SERIAL_UTILS_JSON_UTIL_H_

#include <istream>

#include "base/defs.h"

namespace serial {
namespace util {

// Skip a field in the stream.
void JSONSkipAheadField(istream& in);

// Skip a field in the stream and fill the string representing this field.
// Returns true on success.
bool JSONSkipAheadAndReturnField(istream& in, string* res);

// Skip a field in the stream and returns the string representing this field.
inline string JSONSkipAheadAndReturnField(istream& in) {
  string res;
  JSONSkipAheadAndReturnField(in, &res);
  return res;
}

}  // namespace util
}  // namespace serial


#endif  // _PUBLIC_UTIL_SERIAL_UTILS_JSON_UTIL_H_
