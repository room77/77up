#ifndef _PUBLIC_UTIL_TIME_TIMEUTIL_H_
#define _PUBLIC_UTIL_TIME_TIMEUTIL_H_

#include "util/time/utime.h"
#include "util/time/genericpoint.h"
#include "util/time/timedomain.h"

namespace TimeUtil {

  // check if current time is in the specified time-domain,
  // If inside: return true;
  //            set prev_boundary to the domain's start time
  //              (if it does not exist, set it to LocalTime::InfinitePast())
  //            set next_boundary to the domain's end time
  //              (if it does not exist, set it to LocalTime::InfiniteFuture())
  // If outside: return false;
  //           set prev_boundary to the last domain's end time
  //              (if it does not exist, set it to LocalTime::InfinitePast())
  //           set next_boundary to the next domain's start time
  //              (if it does not exist, set it to LocalTime::InfiniteFuture())
  //
  // In all cases, prev_boundary is guaranteed to be <= current
  //               next_boundary is guaranteed to be > current
  bool CheckDomain(const TimeDomain<SharpPoint>& domain,
                   Time t, Timezone tz,
                   Time *prev_boundary, Time *next_boundary);
  bool CheckDomain(const TimeDomain<GenericPoint>& domain,
                   Time t, const Place& place,
                   Time *prev_boundary, Time *next_boundary);

}

#endif  // _PUBLIC_UTIL_TIME_TIMEUTIL_H_
