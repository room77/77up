#include "util/time/utime.h"

// conversion from universal time to local time
LocalTime Time::GetLocalTime(Timezone tz) const {
  if (IsFinite()) {
    LocalTime result;
    tz.ToLocalTime(t_, &result);
    return result;
  }
  else if (IsInfiniteFuture())
    return LocalTime::InfiniteFuture();
  else if (IsInfinitePast())
    return LocalTime::InfinitePast();
  else {
    return LocalTime::InfinitePast();  // incorrect input
    // ASSERT(false);
    // return LocalTime();  // dummy statement to make compiler happy
  }
}

// conversion from localtime to universal time
Time Time::GetUniversalTime(Timezone tz, const LocalTime& local) {
  if (local.IsFinite()) {
    Time t(tz.FromLocalTime(local));
    return t;
  }
  else if (local.IsInfiniteFuture())
    return InfiniteFuture();
  else if (local.IsInfinitePast())
    return InfinitePast();
  else {
    return InfinitePast();
    // ASSERT(false);
    // return Time();  // dummy statement to make compiler happy
  }
}

// return human-readable time-duration string (5 hours 2 minutes, 5h2m, etc.)
string Time::PrintDuration(int num_seconds) {
  stringstream ss;

  if (num_seconds < 0) {
    num_seconds = -num_seconds;
    ss << "-";
  }

  int days = num_seconds / ONE_DAY;
  int hours = (num_seconds % ONE_DAY) / ONE_HOUR;
  int minutes = (num_seconds % ONE_HOUR) / ONE_MINUTE;
  int seconds = (num_seconds % ONE_MINUTE) / ONE_SECOND;

  if (days > 0)
    ss << days << " day" << (days > 1 ? "s" : "");
  if (hours > 0 || minutes > 0 || seconds > 0)
    ss << " ";

  if (hours > 0)
    ss << hours << " hour" << (hours > 1 ? "s" : "");
  if (minutes > 0 || seconds > 0)
    ss << " ";

  if (minutes > 0)
    ss << minutes << " minute" << (minutes > 1 ? "s" : "");

  if (hours > 0 || minutes > 0)
    ss << " ";

  ss << seconds << " second" << (seconds > 1 ? "s" : "");
  return ss.str();
}

string Time::PrintDurationShort(int num_seconds) {
  stringstream ss;
  if (num_seconds < 0) {
    num_seconds = -num_seconds;
    ss << "-";
  }
  int hours = num_seconds / ONE_HOUR;
  int minutes = (num_seconds % ONE_HOUR) / ONE_MINUTE;
  int seconds = (num_seconds % ONE_MINUTE) / ONE_SECOND;
  if (hours > 0)
    ss << hours << "h";
  if (minutes > 0)
    ss << minutes << "m";
  ss << seconds << "s";
  return ss.str();
}
