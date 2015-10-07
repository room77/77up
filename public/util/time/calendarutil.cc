#include "util/time/calendarutil.h"

namespace CalendarUtil {

  // get the maximum number of days in a month
  int MaxDaysInMonth(int year, int month) {
    static int num_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 0) return 31;  // max. days for any month

    ASSERT(month >= 1 && month <= 12);
    if (month == 2)
      return (IsLeapYear(year) ? 29 : 28);
    else
      return num_days[month - 1];
  }

  // get the day sequence number (1 through 366) for a given date
  int GetDayOfYear(int year, int month, int day) {
    static int day_1_seq[12] = {0, 31, 59, 90, 120, 151,
                                181, 212, 243, 273, 304, 334};
    int seq = day_1_seq[month - 1] + day;
    if (IsLeapYear(year) && month >= 3)
      seq++;
    return seq;
  }

  // precompute day-sequence mapping to (month, day) tuples
  // (for a non-leap year)
  DayNumMapping_NonLeap::DayNumMapping_NonLeap() {
    VLOG(5) << "Calculating day sequence numbers...";
    int seq = 0;
    for (int i = 1; i <= 12; i++) {
      int num_days = MaxDaysInMonth(2007, i);
      for (int j = 1; j <= num_days; j++) {
        month_[seq] = i;
        day_[seq] = j;
        ++seq;
      }
    }
  }

  // the reverse of GetDayOfYear:
  // given a year and a day sequence number, find the exact date (month/day)
  void FindByDayNum(int year, int day_seq, int *month, int *day) {
    static DayNumMapping_NonLeap mapping;  // precomputed mapping
    if (IsLeapYear(year)) {  // leap year: we have 3 cases below
      ASSERT(day_seq >= 1 && day_seq <= 366);
      if (day_seq < 60) {  // January 1 -- February 28
        *month = mapping.GetMonth(day_seq);
        *day = mapping.GetDay(day_seq);
      }
      else if (day_seq == 60) {  // February 29
        *month = 2;
        *day = 29;
      }
      else {  // March 1 -- December 31
        *month = mapping.GetMonth(day_seq - 1);
        *day = mapping.GetDay(day_seq - 1);
      }
    }
    else {  // non-leap year
      ASSERT(day_seq >= 1 && day_seq <= 365);
      *month = mapping.GetMonth(day_seq);
      *day = mapping.GetDay(day_seq);
    }
  }

  // find out how many days have elapsed since January 1, 1970
  int DaysSince1970(int year, int month, int day) {
    if (year < 1970) return 0;

    // first, count how how many days have elapsed between 1/1/1970 and
    // January 1st of the given year
    int num_years = year - 1970;
    // count the number of leap years in between
    int num_4_multiples = (year - 1969) / 4;
    int num_100_multiples = (year - 1901) / 100;
    int num_400_multiples = (year - 1601) / 400;
    int num_leap_years =
      num_4_multiples - num_100_multiples + num_400_multiples;

    int days_until_jan_1 = num_years * 365 + num_leap_years;

    int day_of_year = GetDayOfYear(year, month, day);

    int total_days = days_until_jan_1 + (day_of_year - 1);
    return total_days;
  }

  // reverse of DaysSince1970:
  // from the total number of days after 1/1/1970, find the exact date
  void ElapseFrom1970(int total, int *year, int *month, int *day) {
    int max_years = total / 365;  // at most this many years have elapsed
    int y = 1970 + max_years;
    int d = DaysSince1970(y, 1, 1);
    while (d > total) {
      // try again with an earlier year
      --y;
      d = DaysSince1970(y, 1, 1);
    }
    // now we've found the year
    *year = y;
    // calculate remaining days
    int day_seq = total - d + 1;  // 1 through 365 or 366
    FindByDayNum(y, day_seq, month, day);
  }

  // get the day of week of a particular date
  int GetDayOfWeek(int year, int month, int day) {
    int days_since_1970 = DaysSince1970(year, month, day);

    // January 1, 1970 was a Thursday
    int d = (5 + days_since_1970) % 7;
    if (d == 0)
      d = 7;
    return d;
  }

}

