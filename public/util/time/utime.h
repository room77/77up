#ifndef _PUBLIC_UTIL_TIME_UTIME_H_
#define _PUBLIC_UTIL_TIME_UTIME_H_

#include "base/common.h"
#include "util/time/localtime.h"
#include "util/time/timezone.h"

class Time {
 public:
  // initialize with current time
  Time() { t_ = time(NULL); }

  // initialize with specified time in time_t format
  Time(time_t t) { SetTime(t); };

  // initialize with specified time in broken-down format
  // In case of ambiguity with regard to dst (i.e., during the hour
  // before and the hour after dst ends), "isdst" should be set to:
  // 1: dst, 0: not dst.
  Time(int year, int month, int day, int hour, int minute, int second,
       Timezone tz, int isdst = -1) {
    SetTime(year, month, day, hour, minute, second, tz, isdst);
  }

  // initialize with local time and time zone
  Time(const LocalTime& localtime, Timezone tz) {
    FromLocalTime(localtime, tz);
  }

  ~Time() {};

  inline static Time Now() {  // current system time
    Time now;
    return now;
  }

  inline void FromLocalTime(const LocalTime& localtime, Timezone tz) {
    if (localtime.IsFinite())
      t_ = tz.FromLocalTime(localtime);
    else if (localtime.IsInfinitePast())
      t_ = INFINITE_PAST;
    else
      t_ = INFINITE_FUTURE;
  }

  inline LocalTime ToLocalTime(Timezone tz) const {
    return GetLocalTime(tz);
  }

  inline static LocalTime CurrentTimeInPST() {
    // current local time in Pacific Time (auto-adjust for Daylight Saving Time)
    Time now;
    return now.GetLocalTime(Timezone::PST());
  }

  inline static LocalDate CurrentDateInPST() {
    // current local date in Pacific Time (auto-adjust for Daylight Saving Time)
    Time now;
    return now.GetLocalDate(Timezone::PST());
  }

  inline static LocalTime CurrentTimeInEST() {
    // current local time in Eastern Time (auto-adjust for Daylight Saving Time)
    Time now;
    return now.GetLocalTime(Timezone::EST());
  }

  inline static LocalDate CurrentDateInEST() {  // current local date in EST
    // current local date in Eastern Time (auto-adjust for Daylight Saving Time)
    Time now;
    return now.GetLocalDate(Timezone::EST());
  }

  inline static LocalTime CurrentLocalTime(Timezone tz) {
    // current local time
    Time now;
    return now.GetLocalTime(tz);
  }

  inline static LocalDate CurrentLocalDate(Timezone tz) {
    return LocalDate(CurrentLocalTime(tz));
  }

  // earliest valid time across all possible timezones in the world
  inline static LocalTime EarliestValidTime() {
    Time now;
    now -= (ONE_HOUR * 13);
    return now.GetLocalTime(Timezone::GMT());
  }

  // earliest valid date across all possible timezones in the world
  inline static LocalDate EarliestValidDate() {
    return LocalDate(EarliestValidTime());
  }

  inline static Time InfiniteFuture() {
    static Time t(INFINITE_FUTURE);
    return t;
  }
  inline bool IsInfiniteFuture() const {
    return (t_ == INFINITE_FUTURE);
  }

  inline static Time InfinitePast() {
    static Time t(INFINITE_PAST);
    return t;
  }
  inline bool IsInfinitePast() const {
    return (t_ == INFINITE_PAST);
  }
  inline bool IsFinite() const {
    return (t_ > INFINITE_PAST && t_ < INFINITE_FUTURE);
  }

  // get time in time_t format
  inline time_t t() const     { return t_; }
  inline time_t get_t() const { return t_; }

  // set time in time_t format
  inline void SetTime(time_t t) { t_ = t; }

  // set time in broken-down format
  inline void SetTime(int year, int month, int day,
                      int hour, int minute, int second,
                      Timezone tz, int isdst = -1) {
    LocalTime local(year, month, day, hour, minute, second);
    local.set_is_dst(isdst);
    t_ = tz.FromLocalTime(local);
  }

  // conversion from universal time to local time
  LocalTime GetLocalTime(Timezone tz) const;
  inline LocalDate GetLocalDate(Timezone tz) const {
    return LocalDate(GetLocalTime(tz));
  }

  // conversion from localtime to universal time
  static Time GetUniversalTime(Timezone tz, const LocalTime& local);

  // return the last time when localtime was 00:00:00 (beginning of the day)
  // (return value <= current time)
  inline Time LastMidnight(Timezone tz) const {
    static TimePoint<SharpPoint> td("h0m0s0");
    LocalTime prev;
    td.GetPrevious(GetLocalTime(tz), NULL, &prev);
    return GetUniversalTime(tz, prev);
  }

  // return the next time when localtime was 00:00:00 (beginning of tomorrow)
  // Note: It is not (LastMidnight() + ONE_DAY) !  Think about daylight saving
  //       time changes.
  // (return value > current time)
  inline Time NextMidnight(Timezone tz) const {
    static TimePoint<SharpPoint> td("h0m0s0");
    LocalTime next;
    td.GetNext(GetLocalTime(tz).GetNextSecond(), NULL, &next);
    return GetUniversalTime(tz, next);
  }

  // return the last time when local time was on the hour (hh:00:00)
  inline Time LastFullHour(Timezone tz) const {
    // note: do not use TimePoint "m0s0" here, because TimePoint operates on
    //       LocalTime and does not know about DST changes.
    LocalTime local = GetLocalTime(tz);
    int fraction = local.get_minute() * ONE_MINUTE + local.get_second();
    return ((*this) - fraction);
  }

  // return the next time when local time is on the hour (hh:00:00)
  inline Time NextFullHour(Timezone tz) const {
    // note: do not use TimePoint "m0s0" here, because TimePoint operates on
    //       LocalTime and does not know about DST changes.
    LocalTime local = GetLocalTime(tz);
    int fraction = local.get_minute() * ONE_MINUTE + local.get_second();
    return ((*this) + (ONE_HOUR - fraction));
  }

  // return the time on a specified multiple after a minimum cushion
  // (all units are seconds; "multiple" means the raw time_t data is an integer
  //  multiple of some value)
  inline Time CushionForward(int min_cushion, int multiple) const {
    Time t = (*this) + min_cushion - 1;
    return t.NextMultiple(multiple);
  }
  // return the time on a specified multiple before a minimum cushion
  // (all units are seconds; "multiple" means the raw time_t data is an integer
  //  multiple of some value)
  inline Time CushionBackward(int min_cushion, int multiple) const {
    Time t = (*this) - min_cushion;
    return t.LastMultiple(multiple);
  }

  // return the last time when the time value was a multiple of M
  // (must be <= current time)
  inline Time LastMultiple(int M) const {
    int num_sec = t_ % M;
    return ((*this) - num_sec);
  }

  // return the next time when the time value will be a multiple of M
  // (must be > current time)
  inline Time NextMultiple(int M) const {
    int num_sec = t_ % M;
    return ((*this) - num_sec + M);
  }

  // return ascii representation of the local time
  inline string Print(Timezone tz) const {
    return GetLocalTime(tz).Print();
  }
  inline string PrintDateOnly(Timezone tz) const {
    return GetLocalTime(tz).PrintDateOnly();
  }

  // return human-readable time-duration string (5 hours 2 minutes, 5h2m, etc.)
  static string PrintDuration(int num_seconds);
  static string PrintDurationShort(int num_seconds);

  // overloaded operators
  inline Time operator+(int num_seconds) const {
    Time tmp(t_ + num_seconds);
    return tmp;
  }
  inline Time operator-(int num_seconds) const {
    Time tmp(t_ - num_seconds);
    return tmp;
  }
  inline int operator-(const Time& time2) const { return (t_ - time2.t()); }
  inline bool operator>(const Time& time2) const { return (t_ > time2.t()); }
  inline bool operator<(const Time& time2) const { return (t_ < time2.t()); }
  inline bool operator>=(const Time& time2) const {
    return (t_ >= time2.t());
  }
  inline bool operator<=(const Time& time2) const {
    return (t_ <= time2.t());
  }
  inline bool operator==(const Time& time2) const {
    return (t_ == time2.t());
  }
  inline bool operator!=(const Time& time2) const {
    return (t_ != time2.t());
  }
  inline const Time& operator+=(int num_seconds) {
    t_ += num_seconds;
    return (*this);
  }
  inline const Time& operator-=(int num_seconds) {
    t_ -= num_seconds;
    return (*this);
  }

 private:
  // Do not define any additional member variables!
  // Time class must be kept lightweight.  Only one member variable (t_)
  // is allowed.

  time_t t_;

  // Do not define any additional member variables!
};

#endif  // _PUBLIC_UTIL_TIME_UTIME_H_
