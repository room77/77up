#include "util/time/timeutil.h"

namespace TimeUtil {
  bool CheckDomain(const TimeDomain<SharpPoint>& domain,
                   Time t,
                   Timezone tz,
                   Time *prev_boundary,
                   Time *next_boundary) {
    LocalTime prev, next;
    bool inside = domain.CheckTime(t.GetLocalTime(tz), NULL, &prev, &next);
    *prev_boundary = Time::GetUniversalTime(tz, prev);
    *next_boundary = Time::GetUniversalTime(tz, next);
    return inside;
  }

  bool CheckDomain(const TimeDomain<GenericPoint>& domain,
                   Time t, const Place& place,
                   Time *prev_boundary, Time *next_boundary) {
    Timezone tz = place.get_tz();
    LocalTime prev, next;
    bool inside = domain.CheckTime(t.GetLocalTime(tz), &place, &prev, &next);
    *prev_boundary = Time::GetUniversalTime(tz, prev);
    *next_boundary = Time::GetUniversalTime(tz, next);
    return inside;
  }

}
