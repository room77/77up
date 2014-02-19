#ifndef _PUBLIC_UTIL_TIME_CALENDARUTIL_H_
#define _PUBLIC_UTIL_TIME_CALENDARUTIL_H_

//
// some utilities for calendar calculation
//
// Note: all parameters/results follow GDF 4.0 TimeDomain spec:
//   year: 4-digit integer
//   month: 1 to 12
//   day: 1 to 31
//   day of week: 1: Sunday, 2: Monday, ..., 7: Saturday
//   hour: 0 to 23
//   minute: 0 to 59
//   second: 0 to 59
//
// This is slightly different from "struct tm" definition

#include "base/common.h"

namespace CalendarUtil {
  // check if a particular year is a leap year
  inline bool IsLeapYear(int year) {
    if ((year % 100) == 0)
      return ((year % 400) == 0);
    else
      return ((year % 4) == 0);
  }

  // get the total number of days in a year
  inline int TotalDaysInYear(int year) {
    return (IsLeapYear(year) ? 366 : 365);
  }

  // get the maximum number of days in a month
  int MaxDaysInMonth(int year, int month);

  // get the day sequence number (1 through 366) for a given date
  int GetDayOfYear(int year, int month, int day);

  // a class to precompute day-sequence mapping to (month, day) tuples
  // (for a non-leap year)
  // -- used by FindByDayNum only
  class DayNumMapping_NonLeap {
   public:
    DayNumMapping_NonLeap();
    inline int GetMonth(int d) const { return month_[d - 1]; }
    inline int GetDay(int d) const   { return day_[d - 1]; }
   private:
    int month_[365], day_[365];
  };

  // the reverse of GetDayOfYear:
  // given a year and a day sequence number, find the exact date (month/day)
  void FindByDayNum(int year, int day_seq, int *month, int *day);

  // find out how many days have elapsed since January 1, 1970
  int DaysSince1970(int year, int month, int day);

  // reverse of DaysSince1970:
  // from the total number of days after 1/1/1970, find the exact date
  void ElapseFrom1970(int total, int *year, int *month, int *day);

  // get the day of week of a particular date
  int GetDayOfWeek(int year, int month, int day);

}

#endif  // _PUBLIC_UTIL_TIME_CALENDARUTIL_H_
