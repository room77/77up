// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_UTIL_TIME_DURATION_H_
#define _PUBLIC_UTIL_TIME_DURATION_H_

#include <chrono>
#include <ratio>

namespace util {
namespace time {

// Durations with floating point numbers to record fraction of ticks.
typedef std::chrono::duration<double, std::micro> microsecond;
typedef std::chrono::duration<double, std::milli> millisecond;
typedef std::chrono::duration<double, std::ratio<1>> second;
typedef std::chrono::duration<double, std::ratio<60>> minute;
typedef std::chrono::duration<double, std::ratio<3600>> hour;
typedef std::chrono::duration<double, std::ratio<86400>> day;

}  // namespace time
}  // namespace util


#endif  // _PUBLIC_UTIL_TIME_DURATION_H_
