#ifndef _PUBLIC_UTIL_TIME_TIMEDURATION_H_
#define _PUBLIC_UTIL_TIME_TIMEDURATION_H_

#include "base/common.h"
#include "util/time/localtime.h"

class TimeDuration {
 public:
  TimeDuration() :
    years_(0), months_(0), days_(0), hours_(0), minutes_(0), seconds_(0) {};
  ~TimeDuration() {};

  // initialize from a string
  // (return true if successful; upon return, endptr points to the first
  //  unparsed character)
  bool Init(const char *start, const char **endptr, string *error_msg);

  // given a time (as LocalTime), find the time after this duration
  inline void TimeAfter(const LocalTime& current, LocalTime *after) const {
    ASSERT(current.IsFinite());
    after->Set(current.get_year() + years_,
               current.get_month() + months_,
               current.get_day() + days_,
               current.get_hour() + hours_,
               current.get_minute() + minutes_,
               current.get_second() + seconds_);
  }

  // given a time (as LocalTime), find the time before this duration
  inline void TimeBefore(const LocalTime& current, LocalTime *before) const {
    ASSERT(current.IsFinite());
    before->Set(current.get_year() - years_,
                current.get_month() - months_,
                current.get_day() - days_,
                current.get_hour() - hours_,
                current.get_minute() - minutes_,
                current.get_second() - seconds_);
  }

  // negate every field
  inline void Reverse() {
    years_ = -years_;
    months_ = -months_;
    days_ = -days_;
    hours_ = -hours_;
    minutes_ = -minutes_;
    seconds_ = -seconds_;
  }

  // return a new object with all fields negated
  TimeDuration GetReverse() const {
    TimeDuration t(*this);
    t.Reverse();
    return t;
  }

  // return -1, 0 or 1 depending on the sign of the most significant non-zero
  // field
  int GetSign() const {
    if (years_ > 0) return 1;
    if (years_ < 0) return -1;
    if (months_ > 0) return 1;
    if (months_ < 0) return -1;
    if (days_ > 0) return 1;
    if (days_ < 0) return -1;
    if (hours_ > 0) return 1;
    if (hours_ < 0) return -1;
    if (minutes_ > 0) return 1;
    if (minutes_ < 0) return -1;
    if (seconds_ > 0) return 1;
    if (seconds_ < 0) return -1;
    return 0;
  }

  // for debug: return human-redable string describing this object
  string Print() const;
  string PrintShort() const;

 private:
  // helper function for Print()
  void PrintAppend(int num, const string& name, stringstream& ss) const;

  // parse the next symbol-number representation
  template<class T>
  const char *ParseNext(const char *current, char symbol,
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
    return parse_end;
  }

  int years_, months_, days_, hours_, minutes_, seconds_;
};


//
// operators for LocalTime +/- TimeDuration
//

inline LocalTime operator+(const LocalTime& t, const TimeDuration& d) {
  ASSERT(t.IsFinite());
  LocalTime newtime;
  d.TimeAfter(t, &newtime);
  return newtime;
}

inline LocalTime operator-(const LocalTime& t, const TimeDuration& d) {
  ASSERT(t.IsFinite());
  LocalTime newtime;
  d.TimeBefore(t, &newtime);
  return newtime;
}


#endif  // _PUBLIC_UTIL_TIME_TIMEDURATION_H_
