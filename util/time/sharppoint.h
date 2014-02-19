#ifndef _PUBLIC_UTIL_TIME_SHARPPOINT_H_
#define _PUBLIC_UTIL_TIME_SHARPPOINT_H_

//
// Part of TimeDomain family of classes.
//   (See comments of timedomain.h for details)
//

#include "base/common.h"
#include "util/time/calendarutil.h"
#include "util/time/localtime.h"
#include <time.h>

class SharpPoint {
 public:
  SharpPoint() {
    Clear();
  };
  ~SharpPoint() {};

  static const int UNDEFINED = -100;

  // type of auxiliary data to be passed to GetNext() and GetPrevious()
  // -- void (no additional data needed) for SharpPoint
  // -- Place for FuzzyPoint
  typedef void tAdditionalData;

  void Clear() {
    // set everything to undefined
    year_ = month_ = week_in_year_ = week_in_month_
      = day_in_week_ = day_in_month_ = hour_ = minute_ = second_ = UNDEFINED;
  }

  // initialize from a string
  // (return true if successful; upon return, endptr points to the first
  //  unparsed character)
  bool Init(const char *start, const char **endptr, string *error_msg);

  // for debug: initialize from a string and check for extra characters
  //            after the end
  bool Init(const string& input, string *error_msg) {
    const char *end;
    string error;
    if (Init(input.c_str(), &end, &error)) {
      if (*(strutil::SkipSpaces(end)) != '\0') {
        *error_msg = strutil::ErrorMsg("extra characters at the end",
                                       end - input.c_str() + 1);
        return false;
      }
      else {
        error_msg->clear();
        return true;
      }
    }
    else {
      *error_msg = strutil::ErrorMsg(error, end - input.c_str() + 1);
      return false;
    }
  }

  // return human-redable string describing this object
  string Print() const;
  string PrintShort() const;

  // given a time (as LocalTime), find the prev/next time point that
  // matches this specification
  // -- return true if successful ("next" is set)
  // -- return false if such a time cannot be found
  bool ModifyTime(const LocalTime& current, bool get_next,
                  LocalTime *modified_time) const;
 
  // given a time (as LocalTime), find the next time point that
  // matches this specification  (must be >= current)
  // ("data" parameter is ignored)
  inline bool GetNext(const LocalTime& current, const tAdditionalData *data,
                      LocalTime *next) const {
    VLOG(5) << "Calling GetNext(" << current.Print()
           << " . " << PrintShort() << ")";
    bool exists = ModifyTime(current, true, next);
    VLOG(5) << "GetNext(" << current.Print()
           << " . " << PrintShort() << ") = "
           << (next ? next->Print() : (exists ? "exists" : "does not exist"));
    return exists;
  }

  // given a time (as LocalTime), return the previous time point that
  // matches this specification  (must be <= current)
  // ("data" parameter is ignored)
  inline bool GetPrevious(const LocalTime& current,
                          const tAdditionalData *data,
                          LocalTime *previous) const {
    VLOG(5) << "Calling GetPrevious(" << current.Print()
           << " . " << PrintShort() << ")";
    bool exists = ModifyTime(current, false, previous);
    VLOG(5) << "GetPrevious(" << current.Print()
           << " . " << PrintShort() << ") = "
           << (previous ?
               previous->Print() :
               (exists ? "exists" : "does not exist"));
    return exists;
  }

  // answer questions such as "when is the second Tuesday of this month?"
  // for a given year, month, week_in_month (1 to 5 for forward counting,
  // -1 to -5 for reverse couting) and day_in_week
  int LocateByWeekAndDay(int year, int month,
                         int week_in_month, int day_in_week) const;

 protected:
  // initialize from specification without setting default values
  bool InitFromSpec(const char *start, const char **endptr, string *error_msg);
  // set default values of certain fields
  void SetDefaultValues();

  // parse the next symbol-number representation
  template<class T>
  const char *ParseNext(const char *current, char symbol,
                        int range_begin, int range_end,
                        T *result, string *error_msg) const {
    // parse the symbol before the integer
    ASSERT(symbol != '\0');
    if (*current != symbol) return current;
    ++current;
    if (!IS_DIGIT(*current)) {
      *error_msg = "number expected";
      return current;
    }
    // parse the integer
    char *parse_end;
    *result = strtol(current, &parse_end, 10);
    if (*result >= range_begin && *result <= range_end)
      return parse_end;
    else {
      *error_msg = "number out of range";
      return current;
    }
  }

  // parse "day" representation (four possible formats; see comments in
  // timedomain.h for details)
  const char *ParseDay(const char *current, string *error_msg);

  // parse "n-th day of the x-th week of a month" or
  // "n-th day of the last x-th week of a month"
  const char *ParseWeekDaySpec(const char *current,
                               char *week_output, string *error_msg);

  inline bool AllEmpty() const {
    return (year_ == UNDEFINED &&
            month_ == UNDEFINED &&
            week_in_year_ == UNDEFINED &&
            week_in_month_ == UNDEFINED &&
            day_in_week_ == UNDEFINED &&
            day_in_month_ == UNDEFINED &&
            hour_ == UNDEFINED &&
            minute_ == UNDEFINED &&
            second_ == UNDEFINED);
  }

  int year_;
  char month_;

  // week_in_year_: 1 to 53
  char week_in_year_;

  // week_in_month_: 1 to 5 (forward counting), or -1 to -5 (reverse counting)
  char week_in_month_;

  // day_in_week_: 1 to 7 (1: Sunday, 2: Monday, ..., 7: Saturday)
  char day_in_week_;

  // day_in_month_: 1 to 31
  char day_in_month_;

  char hour_, minute_, second_;

};

#endif  // _PUBLIC_UTIL_TIME_SHARPPOINT_H_
