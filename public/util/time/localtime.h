#ifndef _PUBLIC_UTIL_TIME_LOCALTIME_H_
#define _PUBLIC_UTIL_TIME_LOCALTIME_H_

//
// LocalDate and LocalTime handle date/time manipulations without timezone
// info.
//
// They store broken-down time as year, month, day, hour, minute and second.
//
// Valid value ranges for various fields:
//   year: 1970 and later
//   month: 1 to 12
//   day: 1 to 31
//   hour: 0 to 23
//   minute: 0 to 59
//   second: 0 to 59
//
// Normalization is done automatically if caller sets values outside these
// ranges.  For example, 2/31/2008 will be normalized to 3/2/2008.
//
// (derived values)
//   day of week: 1 to 7 (1: Sunday, 2: Monday, ..., 7: Saturday)
//                       (follows GDF 4.0 TimeDomain format)
//   day of year: 1 to 366
//
//
#include <time.h>
#include <chrono>
#include <sstream>

#include "defs.h"
#include "base/extra_defs.h"
#include "base/common.h"
#include "util/time/calendarutil.h"
#include "util/serial/serializer.h"

// special "time" far into the future (2038)
const int INFINITE_FUTURE = 0x7fffffff;

// special "time" far into the past (100 seconds after the beginning)
// (the first 100 seconds may be reserved for other purposes)
const int INFINITE_PAST = 100;


class LocalDate {
 public:
  // empty initialization
  LocalDate() {
    *this = InfinitePast();
  }

  // initialize from components
  LocalDate(int year, int month, int day) :
    y_(year), m_(month), d_(day) {
    if (year != 0 || month != 0 || day != 0)
      NormalizeDate();
  };
  ~LocalDate() {};

  //
  // InfinitePast / InfiniteFuture definitions
  //

  inline static LocalDate InfinitePast() {
    static LocalDate p(1970, 1, 1);
    return p;
  }

  inline static LocalDate InfiniteFuture() {
    static LocalDate f(3000, 1, 1);
    return f;
  }

  inline bool IsInfiniteFuture() const {
    return (y_ >= 3000);
  }
  inline bool IsInfinitePast() const {
    return (y_ <= 1970);
  }
  inline bool IsFinite() const {
    return (y_ > 1970 && y_ < 3000);
  }
  inline bool IsInfinite() const {
    return (!IsFinite());
  }

  static LocalDate ParseFromYYYYMMDD(const string& yyyymmdd, const char delimiter=0);
  static LocalDate ParseFromMMDDYYYY(const string& mmddyyyy, const char delimiter=0);
  static LocalDate Today();

  static const int NO_CHANGE = -INFINITY_INT;

  inline int get_year() const   { return y_; }
  inline int get_month() const  { return m_; }
  inline int get_day() const    { return d_; }

  inline void set_year(int year)     { y_ = year; }
  inline void set_month(int month)   { m_ = month; }
  inline void set_day(int day)       { d_ = day; }

  inline int GetDayOfWeek() const {
    return CalendarUtil::GetDayOfWeek(y_, m_, d_);
  }
  inline int GetDayOfYear() const {
    return CalendarUtil::GetDayOfYear(y_, m_, d_);
  }

  inline string GetDayOfWeek_Abbr() const {
    return DayOfWeek_EnglishAbbr(GetDayOfWeek() - 1);
  }
  inline string GetMonth_Abbr() const {
    return Month_EnglishAbbr(get_month() - 1);
  }

  static string DayOfWeek_EnglishAbbr(int index);
  static string Month_EnglishAbbr(int index);

  inline void Set(int year, int month, int day) {
    // set year/month/day/hour/minute/second components in one call
    if (year != NO_CHANGE)   y_ = year;
    if (month != NO_CHANGE)  m_ = month;
    if (day != NO_CHANGE)    d_ = day;
    NormalizeDate();
  }

  inline void Set(const LocalDate& date) {
    Set(date.y_, date.m_, date.d_);
  }

  inline int DaysSince1970() const {
    return CalendarUtil::DaysSince1970(y_, m_, d_);
  }

  // reverse of DaysSince1970:
  // from the total number of days after 1/1/1970 00:00:00, find the
  // exact date
  inline void ElapseDaysFrom1970(int num_days) {
    CalendarUtil::ElapseFrom1970(num_days, &y_, &m_, &d_);
  }

  //
  // methods to support modification by component
  //   ("increment" may be positive or negative)
  //
  inline void IncrementYear(int increment) {
    y_ += increment;
    NormalizeDate();
  }
  inline void IncrementMonth(int increment) {
    m_ += increment;
    NormalizeDate();
  }
  inline void IncrementDay(int increment) {
    d_ += increment;
    NormalizeDate();
  }

  // Prints the date according to the specified format.
  // The format is specified similar to strftime(...).
  // Read more about strftime formats at:
  // http://www.cplusplus.com/reference/clibrary/ctime/strftime/
  // Note: the specifiers implemented currently are a subset of those specified
  // in strftime.
  // A few differences are liste below:
  // %A: Upper case abbreviated weekday name.
  // %B: Upper case abbreviated month name.
  string PrintFormatted(const string& format) const;

  // return a human-readable string describing the date only
  string PrintDateOnly() const {
    stringstream ss;
    if (IsFinite()) {
      ss << m_ << "/" << d_ << "/" << y_;
      return ss.str();
    }
    else if (IsInfinitePast())
      return "Infinite Past";
    else if (IsInfiniteFuture())
      return "Infinite Future";
    else
      return "**error**";
  }

  // return date string in yyyymmdd format
  string PrintDate_YYYYMMDD(const char* delim = "") const {
    return PrintFormatted(string("%Y") + delim + "%m" + delim + "%d");
  }

  string PrintDate_MMDDYYYY(const char* delim = "") const {
    return PrintFormatted(string("%m") + delim + "%d" + delim + "%Y");
  }

  // European style dates, eg. 31/12/1989
  string PrintDate_DDMMYYYY(const char* delim = "") const {
    return PrintFormatted(string("%d") + delim + "%m" + delim + "%Y");
  }

  string PrintDate_M_D() const {
    return PrintFormatted("%m/%d");
  }

  string PrintDate_Mname_D() const {
    return PrintFormatted("%b %d");
  }

  string PrintDate_DW_M_D() const {
    return PrintFormatted("%a %m/%d");
  }

  string PrintDate_DW_M_D_Y() const {
    return PrintFormatted("%a %m/%d/%Y");
  }

  // format the date as 01JAN99  -- for Galileo PNR generation
  string PrintDate_DDMMMYY() const {
    return PrintFormatted("%d%B%y");
  }

  // format the date as 01JAN  -- for Galileo PNR generation
  string PrintDate_DDMMM() const {
    return PrintFormatted("%d%B");
  }

  // normalize all fields so that all numbers are within range
  void NormalizeDate();

  // helper function for normalization methods:
  //    normalize one field to [0, range_max)
  //    and send carry information to another field
  inline void NormalizeField(int range_max,
                             int& to_normalize, int& carry) {
    if (to_normalize < 0 || to_normalize >= range_max) {
      ASSERT_DEV(range_max > 0);
      int normalized = MOD(to_normalize, range_max);
      int delta = to_normalize - normalized;
      to_normalize = normalized;
      carry += (delta / range_max);
      ASSERT_DEV(to_normalize >= 0 && to_normalize < range_max);
    }
  }

  size_t Hash() const {
    return std::hash<int>()(y_) ^ std::hash<int>()(m_) ^
      std::hash<int>()(d_);
  }

  inline bool SameDay(const LocalDate& other) const {
    return (d_ == other.get_day() &&
            m_ == other.get_month() &&
            y_ == other.get_year());
  }

  // Utility Operators.

  inline LocalDate operator+(int num_days) const {
    LocalDate newdate = *this;
    newdate.IncrementDay(num_days);
    return newdate;
  }

  inline LocalDate operator-(int num_days) const {
    LocalDate newdate = *this;
    newdate.IncrementDay(-num_days);
    return newdate;
  }

  inline int operator-(const LocalDate& other) const {
    return (DaysSince1970() - other.DaysSince1970());
  }

  inline LocalDate& operator+=(int num_days) {
    IncrementDay(num_days);
    return *this;
  }

  inline LocalDate& operator-=(int num_days) {
    IncrementDay(-num_days);
    return *this;
  }

  // Pre increment.
  inline LocalDate& operator++() {
    IncrementDay(1);
    return *this;
  }

  // Pre decrement.
  inline LocalDate& operator--() {
    IncrementDay(-1);
    return *this;
  }

  // Note: We de not support post inc, decr for now.

  bool operator<(const LocalDate& other) const {
    // all fields are already normalized, so we can just compare them directly
    if (y_ < other.get_year()) return true;
    if (y_ > other.get_year()) return false;
    if (m_ < other.get_month()) return true;
    if (m_ > other.get_month()) return false;
    if (d_ < other.get_day()) return true;
    return false;
  }

  bool operator<=(const LocalDate& other) const {
    // all fields are already normalized, so we can just compare them directly
    if (y_ < other.get_year()) return true;
    if (y_ > other.get_year()) return false;
    if (m_ < other.get_month()) return true;
    if (m_ > other.get_month()) return false;
    if (d_ <= other.get_day()) return true;
    return false;
  }

  inline bool operator>(const LocalDate& other) const {
    return (other < *this);
  }

  inline bool operator>=(const LocalDate& other) const {
    return (other <= *this);
  }

  inline bool operator==(const LocalDate& other) const {
    // all fields are already normalized, so we can just compare them directly
    return (y_ == other.get_year() &&
            m_ == other.get_month() &&
            d_ == other.get_day());
  }

  inline bool operator!=(const LocalDate& other) const {
    return !(*this == other);
  }

 protected:
  // Writes the value for the specifier to the out stream.
  // Returns true if it is able to pass the specifier and false otherwise.
  bool PrintSpecifier(char specifier, ostream& out) const;

  // note: all fields are all 4-bytes integers, not 2-byte short or 1-byte char
  //       types, because we want to allow the flexibility of performing
  //       arithmetic operations on them, such as LocalTime +/- TimeDuration.
  int y_, m_, d_;

 public:
  // Called after deserialization.
  bool DeserializationCallback() { NormalizeDate(); return true; }

  SERIALIZE(y_*1 / m_*2 / d_*3);
};


class LocalTime : public LocalDate {
 public:
  // empty initialization
  LocalTime() {
    *this = InfinitePast();
  }

  // initialize from "struct tm" format
  LocalTime(const struct tm *t) {
    FromTM(t);
  }

  // initialize from LocalDate object
  LocalTime(const LocalDate& date) :
    LocalDate(date),
    h_(0), mn_(0), s_(0), is_dst_(-1) {
    Normalize();
  };

  // initialize from LocalDate object and hour/minute/second
  LocalTime(const LocalDate& date, int hour, int minute, int second) :
    LocalDate(date),
    h_(hour), mn_(minute), s_(second), is_dst_(-1) {
    Normalize();
  };

  // initialize from components
  LocalTime(int year, int month, int day, int hour, int minute, int second) :
    LocalDate(year, month, day),
    h_(hour), mn_(minute), s_(second), is_dst_(-1) {
    Normalize();
  };
  ~LocalTime() {};

  // get a localtime representing the current time
  static LocalTime Now() {
    time_t rawtime;
    time(&rawtime);
    struct tm* timeinfo = localtime(&rawtime);
    return LocalTime(timeinfo);
  }

  // Given a duration, returns the UTC time for it.
  template<typename T = chrono::microseconds>
  static LocalTime UTCTimeFromDuration(const T& duration) {
    std::time_t rawtime = std::chrono::system_clock::to_time_t(
        chrono::system_clock::time_point(duration));
    struct tm* timeinfo = std::gmtime(&rawtime);
    return LocalTime(timeinfo);
  }

  // Returns the UTC time from a timestamp (in microseconds).
  static LocalTime UTCTimeFromTimeStamp(uint64_t timestamp) {
    return UTCTimeFromDuration(chrono::microseconds(timestamp));
  }

  //
  // InfinitePast / InfiniteFuture definitions
  //

  inline static LocalTime InfinitePast() {
    static LocalTime p(1970, 1, 1, 0, 0, 0);
    return p;
  }

  inline static LocalTime InfiniteFuture() {
    static LocalTime f(3000, 1, 1, 0, 0, 0);
    return f;
  }

  inline bool IsInfiniteFuture() const {
    return (y_ >= 3000);
  }
  inline bool IsInfinitePast() const {
    return (y_ <= 1970);
  }
  inline bool IsFinite() const {
    return (y_ > 1970 && y_ < 3000);
  }
  inline bool IsInfinite() const {
    return (!IsFinite());
  }

  // convert from/to unix "struct tm" format
  void FromTM(const struct tm *t);
  void ToTM(struct tm *t) const;

  static LocalTime ParseFromYYYYMMDDHHMM(const string& yyyymmdd,
                                         const string& hhmm = "0");

  inline int get_hour() const   { return h_; }
  inline int get_minute() const { return mn_; }
  inline int get_second() const { return s_; }
  inline int is_dst() const     { return is_dst_; }

  inline void set_hour(int hour)     { h_ = hour; }
  inline void set_minute(int minute) { mn_ = minute; }
  inline void set_second(int second) { s_ = second; }
  inline void set_is_dst(int dst)  { is_dst_ = dst; }

  inline void Set(int year, int month, int day,
                  int hour, int minute, int second) {
    // set year/month/day/hour/minute/second components in one call
    if (year != NO_CHANGE)   y_ = year;
    if (month != NO_CHANGE)  m_ = month;
    if (day != NO_CHANGE)    d_ = day;
    if (hour != NO_CHANGE)   h_ = hour;
    if (minute != NO_CHANGE) mn_ = minute;
    if (second != NO_CHANGE) s_ = second;
    Normalize();
  }

  // find out how many seconds have elapsed since January 1, 1970, 00:00:00.
  // NOTE: Do not use this function as a proxy for unix timestamps.
  // This does not take into account localtime to GMT conversion.
  // For unix timestamps use, ::util::Timestamp::Now<chrono::milliseconds>()
  inline time_t SecondsSince1970() const {
    ASSERT_DEV(IsFinite());
    return (DaysSince1970() * ONE_DAY +
            h_ * ONE_HOUR + mn_ * ONE_MINUTE + s_);
  }

  // reverse of SecondsSince1970:
  // from the total number of seconds after 1/1/1970 00:00:00, find the
  // exact time
  void ElapseFrom1970(time_t num_seconds) {
    int num_days = num_seconds / ONE_DAY;
    ElapseDaysFrom1970(num_days);
    int t = num_seconds % ONE_DAY;
    h_ = t / ONE_HOUR;
    t %= ONE_HOUR;
    mn_ = t / ONE_MINUTE;
    t %= ONE_MINUTE;
    s_ = t;
  }

  // return the last time when the time value was a multiple of M
  // (must be <= current time)
  inline LocalTime LastMultiple(int M) const {
    int num_sec = SecondsSince1970() % M;
    return ((*this) - num_sec);
  }

  // return the next time when the time value will be a multiple of M
  // (must be > current time)
  inline LocalTime NextMultiple(int M) const {
    int num_sec = SecondsSince1970() % M;
    return ((*this) - num_sec + M);
  }

  // return the time on a specified multiple after a minimum cushion
  // (all units are seconds; "multiple" means the raw time_t data is an integer
  //  multiple of some value)
  inline LocalTime CushionForward(int min_cushion, int multiple) const {
    LocalTime t = (*this) + min_cushion - 1;
    return t.NextMultiple(multiple);
  }
  // return the time on a specified multiple before a minimum cushion
  // (all units are seconds; "multiple" means the raw time_t data is an integer
  //  multiple of some value)
  inline LocalTime CushionBackward(int min_cushion, int multiple) const {
    LocalTime t = (*this) - min_cushion;
    return t.LastMultiple(multiple);
  }

  //
  // methods to support modification by component
  //   ("increment" may be positive or negative)
  //
  inline void IncrementHour(int increment) {
    h_ += increment;
    Normalize();
  }
  inline void IncrementMinute(int increment) {
    mn_ += increment;
    Normalize();
  }
  inline void IncrementSecond(int increment) {
    s_ += increment;
    Normalize();
  }

  // get another LocalTime object that is one second away from this object
  inline LocalTime GetNextSecond() const {
    LocalTime n = *this;
    n.IncrementSecond(1);
    return n;
  }
  inline LocalTime GetPrevSecond() const {
    LocalTime n = *this;
    n.IncrementSecond(-1);
    return n;
  }

  inline LocalTime LastMidnight() const {
    LocalTime t(*this);
    t.set_hour(0);
    t.set_minute(0);
    t.set_second(0);
    return t;
  }

  // move forward to the given hour/minute/second (result must be >= *this)
  inline LocalTime ForwardTo(int hour, int minute, int second) const {
    LocalTime t(y_, m_, d_, hour, minute, second);
    if (t < *this)
      t.IncrementDay(1);
    return t;
  }

  // move backward to the given hour/minute/second (result must be <= *this)
  inline LocalTime RewindTo(int hour, int minute, int second) const {
    LocalTime t(y_, m_, d_, hour, minute, second);
    if (t > *this)
      t.IncrementDay(-1);
    return t;
  }

  // return a human-readable string describing the time
  string Print() const {
    stringstream ss;
    if (IsFinite()) {
      ss << m_ << "/" << d_ << "/" << y_ << " "
         << setw(2) << setfill('0') << h_ << ":"
         << setw(2) << setfill('0') << mn_ << ":"
         << setw(2) << setfill('0') << s_;
      return ss.str();
    }
    else if (IsInfinitePast())
      return "Infinite Past";
    else if (IsInfiniteFuture())
      return "Infinite Future";
    else
      return "**error**";
  }

  // print in MySQL format: YYYY-MM-DD HH:MM:SS
  string PrintMySQL() const {
    stringstream ss;
    if (IsFinite()) {
      ss << setw(4) << setfill('0') << y_ << "-"
         << setw(2) << setfill('0') << m_ << "-"
         << setw(2) << setfill('0') << d_ << " "
         << setw(2) << setfill('0') << h_ << ":"
         << setw(2) << setfill('0') << mn_ << ":"
         << setw(2) << setfill('0') << s_;
      return ss.str();
    }
    else
      return "**error**";
  }

  // return time string in hhmm format
  string PrintTime_HHMM() const {
    ASSERT_DEV(IsFinite()) << "Invalid time: " << Print();
    stringstream ss;
    ss << setw(2) << setfill('0') << h_
       << setw(2) << setfill('0') << mn_;
    return ss.str();
  }
  string PrintTime_HHMMSS(const string& DELIMITER = "") const {
    ASSERT_DEV(IsFinite()) << "Invalid time: " << Print();
    stringstream ss;
    ss << setw(2) << setfill('0') << h_ << DELIMITER
       << setw(2) << setfill('0') << mn_ << DELIMITER
       << setw(2) << setfill('0') << s_;
    return ss.str();
  }
  // return date/time string in yyyymmdd.hhmm format
  inline string Print_YYYYMMDD_HHMM() const {
    return PrintDate_YYYYMMDD() + "." + PrintTime_HHMM();
  }

  string PrintTime_H_M_12() const {
    ASSERT_DEV(IsFinite()) << "Invalid time: " << PrintDateOnly();
    stringstream ss;
    if (h_ < 12)
      ss << (h_ == 0 ? 12 : h_) << ":"
         << setw(2) << setfill('0') << mn_ << "a";
    else
      ss << (h_ == 12 ? 12 : (h_ - 12)) << ":"
         << setw(2) << setfill('0') << mn_ << "p";
    return ss.str();
  }

  inline string Print_DW_M_D_Y_H_M_12() const {
    return PrintDate_DW_M_D_Y() + " " + PrintTime_H_M_12();
  }

  // normalize all fields so that all numbers are within range
  void Normalize();

  inline LocalTime DaysAfter(int num_days) const {
    LocalTime newtime = *this;
    newtime.IncrementDay(num_days);
    return newtime;
  }

  inline LocalTime DaysBefore(int num_days) const {
    LocalTime newtime = *this;
    newtime.IncrementDay(-num_days);
    return newtime;
  }

  size_t Hash() const {
    return std::hash<int>()(y_) ^ std::hash<int>()(m_) ^
      std::hash<int>()(d_) ^ std::hash<int>()(h_) ^
      std::hash<int>()(mn_) ^ std::hash<int>()(s_) ^ std::hash<int>()(is_dst_);
  }

  inline LocalTime operator+(int num_seconds) const {
    LocalTime newtime = *this;
    newtime.IncrementSecond(num_seconds);
    return newtime;
  }

  inline LocalTime operator-(int num_seconds) const {
    LocalTime newtime = *this;
    newtime.IncrementSecond(-num_seconds);
    return newtime;
  }

  inline int operator-(const LocalTime& other) const {
    int diff = SecondsSince1970() - other.SecondsSince1970();
    return diff;
  }

  inline LocalTime& operator+=(int num_seconds) {
    IncrementSecond(num_seconds);
    return *this;
  }

  inline LocalTime& operator-=(int num_seconds) {
    IncrementSecond(-num_seconds);
    return *this;
  }

  bool operator<(const LocalTime& other) const {
    // all fields are already normalized, so we can just compare them directly
    if (y_ < other.get_year()) return true;
    if (y_ > other.get_year()) return false;
    if (m_ < other.get_month()) return true;
    if (m_ > other.get_month()) return false;
    if (d_ < other.get_day()) return true;
    if (d_ > other.get_day()) return false;
    if (h_ < other.get_hour()) return true;
    if (h_ > other.get_hour()) return false;
    if (mn_ < other.get_minute()) return true;
    if (mn_ > other.get_minute()) return false;
    if (s_ < other.get_second()) return true;
    return false;
  }
  bool operator<=(const LocalTime& other) const {
    // all fields are already normalized, so we can just compare them directly
    if (y_ < other.get_year()) return true;
    if (y_ > other.get_year()) return false;
    if (m_ < other.get_month()) return true;
    if (m_ > other.get_month()) return false;
    if (d_ < other.get_day()) return true;
    if (d_ > other.get_day()) return false;
    if (h_ < other.get_hour()) return true;
    if (h_ > other.get_hour()) return false;
    if (mn_ < other.get_minute()) return true;
    if (mn_ > other.get_minute()) return false;
    if (s_ <= other.get_second()) return true;
    return false;
  }
  inline bool operator>(const LocalTime& other) const {
    return (other < *this);
  }
  inline bool operator>=(const LocalTime& other) const {
    return (other <= *this);
  }
  inline bool SameLocalTime(const LocalTime& other) const {
    // compare all fields except is_dst_
    // all fields are already normalized, so we can just compare them directly
    return (y_ == other.get_year() &&
            m_ == other.get_month() &&
            d_ == other.get_day() &&
            h_ == other.get_hour() &&
            mn_ == other.get_minute() &&
            s_ == other.get_second());
  }
  inline bool operator==(const LocalTime& other) const {
    // compare all fields including is_dst_ (if set)
    return (SameLocalTime(other) &&
            (is_dst_ < 0 || other.is_dst() < 0 || is_dst_ == other.is_dst()));
  }
  inline bool operator!=(const LocalTime& other) const {
    return !(*this == other);
  }

 protected:

  // note: all fields are all 4-bytes integers, not 2-byte short or 1-byte char
  //       types, because we want to allow the flexibility of performing
  //       arithmetic operations on them, such as LocalTime +/- TimeDuration.
  int h_, mn_, s_;
  int is_dst_;  // 1: DST is in effect; 0: DST is not in effect; -1: unknown

 public:
  SERIALIZE(y_*1 / m_*2 / d_*3 / h_*4 / mn_*5 / s_*6 / is_dst_*7);
};

#endif  // _PUBLIC_UTIL_TIME_LOCALTIME_H_
