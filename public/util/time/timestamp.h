// Copyright 2013 Room 77, Inc.
// Author: Nicholas Edelman (edelman)

//
// basic utility functions for computing timestamps of
// different resolutions
//

#ifndef _PUBLIC_UTIL_TIME_TIMESTAMP_H_
#define _PUBLIC_UTIL_TIME_TIMESTAMP_H_

#include "base/common.h"

namespace util {

class Timestamp {
 public:
  // get the current timestamp
  // for seconds use Now<chrono::seconds>
  // for milliseconds use Now<chrono::milliseconds>
  // for microseconds use Now<chrono::microseconds>
  template<typename Resolution = chrono::microseconds>
  static uint64_t Now() {
    return chrono::duration_cast<Resolution>(
        chrono::high_resolution_clock::now().time_since_epoch()).count();
  }

  // get the timestamp of the previous midnight
  static std::chrono::high_resolution_clock::time_point PrevMidnight() {
    auto now = std::chrono::high_resolution_clock::now();
    time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm *date = std::localtime(&tnow);
    date->tm_hour = 0;
    date->tm_min = 0;
    date->tm_sec = 0;
    return std::chrono::system_clock::from_time_t(std::mktime(date));
  }

  // get the timestamp of the previous midnight
  template<typename Resolution = chrono::microseconds>
  static uint64_t PrevMidnightTimeStamp() {
    return chrono::duration_cast<Resolution>(PrevMidnight().time_since_epoch()).count();
  }

};

} // namespace util

#endif  // _PUBLIC_UTIL_TIME_TIMESTAMP_H_
