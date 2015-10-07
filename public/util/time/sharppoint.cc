#include "util/time/sharppoint.h"

// initialize from a string
// (return true if successful; upon return, endptr points to the first
//  unparsed character)
bool SharpPoint::Init(const char *start, const char **endptr,
                      string *error_msg) {
  error_msg->clear();

  const char *first_char = strutil::SkipSpaces(start);

  if (InitFromSpec(first_char, endptr, error_msg)) {
    SetDefaultValues();

    VLOG(5) << "Parsed \"" << string(start, *endptr - start)
           << "\" as: " << Print();
    return true;
  }
  else
    return false;
}


// initialize from specification without setting default values
bool SharpPoint::InitFromSpec(const char *start, const char **endptr,
                              string *error_msg) {
  *endptr = ParseNext(start, 'y', 1970, INFINITY_INT, &year_, error_msg);
  if (!error_msg->empty()) return false;

  *endptr = ParseNext(*endptr, 'M', 1, 12, &month_, error_msg);
  if (!error_msg->empty()) return false;

  if (month_ != UNDEFINED) {
    // user specified a valid month
    // now proceed to parse "day" spec
    *endptr = ParseDay(*endptr, error_msg);
    if (!error_msg->empty()) return false;
  }
  else {
    // user did not specify a month
    // attempt to parse "week" spec first
    *endptr = ParseNext(*endptr, 'w', 1, 53, &week_in_year_, error_msg);
    if (!error_msg->empty()) return false;

    if (week_in_year_  != UNDEFINED) {
      // user specified a valid week

      // attempt to parse "day of week" spec
      // (no other "day" specs are valid, given that "week" is specified)
      *endptr = ParseNext(*endptr, 't', 1, 7, &day_in_week_, error_msg);
      if (!error_msg->empty()) return false;
    }
    else {
      // user did not specify either week or month

      // proceed to parse "day" spec
      *endptr = ParseDay(*endptr, error_msg);
      if (!error_msg->empty()) return false;
    }
  }

  // parse hour, minute and second

  *endptr = ParseNext(*endptr, 'h', 0, 23, &hour_, error_msg);
  if (!error_msg->empty()) return false;

  *endptr = ParseNext(*endptr, 'm', 0, 59, &minute_, error_msg);
  if (!error_msg->empty()) return false;

  *endptr = ParseNext(*endptr, 's', 0, 59, &second_, error_msg);
  if (!error_msg->empty()) return false;

  return true;
}


// set default values of certain fields
void SharpPoint::SetDefaultValues() {
  if (AllEmpty()) return;

  // set default values:
  //   a field will be filled with default value if:
  //     - it's unspecified, and
  //     - all fields with smaller units are also specified
  //     - at least one field is specified
  if (second_ == UNDEFINED) {
    second_ = 0;
    if (minute_ == UNDEFINED) {
      minute_ = 0;
      if (hour_ == UNDEFINED) {
        hour_ = 0;
        if (day_in_month_ == UNDEFINED && day_in_week_ == UNDEFINED &&
            week_in_month_ == UNDEFINED) {
          if (week_in_year_ != UNDEFINED)
            day_in_week_ = 1;
          else
            day_in_month_ = 1;

          if (week_in_year_ == UNDEFINED && month_ == UNDEFINED) {
            month_ = 1;

            // we already know that the input is non-empty, so at least
            // one field is specified
            ASSERT(year_ != UNDEFINED);
          }
        }
      }
    }
  }
}

// parse "day" representation (four possible formats; see comments in
// timedomain.h for details)
const char *SharpPoint::ParseDay(const char *current,
                                 string *error_msg) {
  switch (*current) {
  case 'd': {
    // parse day of month
    current = ParseNext(current, 'd', 1,
                        CalendarUtil::MaxDaysInMonth(year_, month_),
                        &day_in_month_, error_msg);
    return current;
  }
  case 'f': {
    return ParseWeekDaySpec(current, &week_in_month_, error_msg);
  }
  case 'l': {
    const char *ret = ParseWeekDaySpec(current,
                                       &week_in_month_, error_msg);
    week_in_month_ = -week_in_month_;
    return ret;
  }
  case 't': {
    // parse day of week
    current = ParseNext(current, 't', 1, 7, &day_in_week_, error_msg);
    return current;
  }
  }
  return current;
}


// return human-redable string describing this object
string SharpPoint::Print() const {
  stringstream ss;
  if (year_ == UNDEFINED)
    ss << "any year, ";
  else
    ss << "year " << year_ << ", ";

  if (month_ != UNDEFINED) {
    ASSERT(week_in_year_ == UNDEFINED);
    ss << "month " << static_cast<int>(month_) << ", ";
  }
  else if (week_in_year_ != UNDEFINED) {
    ASSERT(week_in_month_ == UNDEFINED);
    ASSERT(day_in_month_ == UNDEFINED);
    ss << "week " << static_cast<int>(week_in_year_) << ", ";
  }
  else
    ss << "any month, ";

  if (week_in_month_ != UNDEFINED) {
    static const char *count_txt[11] = {
      "5th-to-last", "4th-to-last", "3rd-to-last", "2nd-to-last", "last", "**",
      "1st", "2nd", "3rd", "4th", "5th"
    };
    ASSERT(week_in_month_ >= -5 && week_in_month_ <= 5 && week_in_month_ != 0);
    ASSERT(day_in_week_ != UNDEFINED);
    ss << count_txt[week_in_month_ + 5] << " ";
  }
  if (day_in_week_ != UNDEFINED) {
    static const char *wday_name[7] = {
      "Sunday", "Monday", "Tuesday", "Wednesday",
      "Thursday", "Friday", "Saturday"
    };
    ASSERT(day_in_week_ >= 1 && day_in_week_ <= 7);
    ss << wday_name[day_in_week_ - 1] << ", ";
  }
  if (day_in_month_ != UNDEFINED) {
    ASSERT(week_in_month_ == UNDEFINED);
    ASSERT(week_in_year_ == UNDEFINED);
    ss << "day " << static_cast<int>(day_in_month_) << ", ";
  }

  if (hour_ != UNDEFINED)
    ss << setw(2) << setfill('0') << static_cast<int>(hour_) << ":";
  else
    ss << "**:";

  if (minute_ != UNDEFINED)
    ss << setw(2) << setfill('0') << static_cast<int>(minute_) << ":";
  else
    ss << "**:";

  if (second_ != UNDEFINED)
    ss << setw(2) << setfill('0') << static_cast<int>(second_);
  else
    ss << "**";

  return ss.str();
}

string SharpPoint::PrintShort() const {
  stringstream ss;
  if (year_ != UNDEFINED)
    ss << "y" << year_;

  if (month_ != UNDEFINED) {
    ASSERT(week_in_year_ == UNDEFINED);
    ss << "M" << static_cast<int>(month_);
  }
  else if (week_in_year_ != UNDEFINED) {
    ASSERT(week_in_month_ == UNDEFINED);
    ASSERT(day_in_month_ == UNDEFINED);
    ss << "w" << static_cast<int>(week_in_year_);
  }

  if (week_in_month_ != UNDEFINED) {
    ASSERT(week_in_month_ >= -5 && week_in_month_ <= 5 && week_in_month_ != 0);
    ASSERT(day_in_week_ != UNDEFINED);
    if (week_in_month_ > 0)
      ss << "f" << static_cast<int>(week_in_month_)
         << static_cast<int>(day_in_week_);
    else
      ss << "l" << static_cast<int>(-week_in_month_)
         << static_cast<int>(day_in_week_);
  }
  else if (day_in_week_ != UNDEFINED) {
    ASSERT(day_in_week_ >= 1 && day_in_week_ <= 7);
    ss << "t" << static_cast<int>(day_in_week_);
  }
  else if (day_in_month_ != UNDEFINED) {
    ASSERT(week_in_month_ == UNDEFINED);
    ASSERT(week_in_year_ == UNDEFINED);
    ss << "d" << static_cast<int>(day_in_month_);
  }

  if (hour_ != UNDEFINED)
    ss << "h" << static_cast<int>(hour_);

  if (minute_ != UNDEFINED)
    ss << "m" << static_cast<int>(minute_);

  if (second_ != UNDEFINED)
    ss << "s" << static_cast<int>(second_);

  return ss.str();
}


// parse "n-th day of the x-th week of a month" or
// "n-th day of the last x-th week of a month"
const char *SharpPoint::ParseWeekDaySpec(const char *current,
                                         char *week_output,
                                         string *error_msg) {
  if (IS_DIGIT(*(current + 1)) && IS_DIGIT(*(current + 2))) {
    ++current;
    *week_output = (*current) - '0';
    if (*week_output < 1 || *week_output > 5)
      *error_msg = "number out of range";
    else {
      ++current;
      day_in_week_ = (*current) - '0';
      if (day_in_week_ < 1 || day_in_week_ > 7)
        *error_msg = "number out of range";
      else
        ++current;
    }
  }
  else
    *error_msg = "syntax error";

  return current;
}

// given a time (as LocalTime), find the prev/next time point that
// matches this specification
//   * if get_next is true: find the next time point
//   * if get_next is false: find the previous time point
// -- return true if successful ("next" is set)
// -- return false if such a time cannot be found (newtime is set
//    to either LocalTime::InfiniteFuture() or LocalTime::InfinitePast()
bool SharpPoint::ModifyTime(const LocalTime& current,
                            bool get_next,
                            LocalTime *modified_time) const {
  ASSERT(current.IsFinite());
  bool get_previous = !get_next;  // add another boolean for better readability
  bool done = false;

  LocalTime newtime = current;

  while (!done) {

    VLOG(5) << newtime.Print();

    //
    // adjust "year" according to instructions
    //

    int current_year   = newtime.get_year();
    int current_month  = newtime.get_month();
    int current_day    = newtime.get_day();
    int current_hour   = newtime.get_hour();
    int current_minute = newtime.get_minute();
    int current_second = newtime.get_second();

    if (current_year <= 1970)
      break;  // cannot go earlier than 1970

    if (year_ != UNDEFINED && year_ != current_year) {
      // "year" needs to be changed
      if (year_ > current_year) {
        // advance to a later year, to January 1, 0:00:00
        if (get_previous)
          break;  // impossible to get previous
        else {
          newtime.Set(year_, 1, 1, 0, 0, 0);
          continue;
        }
      }
      else {  // year_ < current_year
        // return to an earlier year, to December 31, 23:59:59
        // (or to January 1 00:00:00 if default_month is true)
        if (get_next)
          break;  // impossible to get next
        else {
          newtime.Set(year_, 12, 31, 23, 59, 59);
          continue;
        }
      }
      // do nothing if we remain in the same year
    }

    //
    // adjust "month" according to instructions
    //

    if (month_ != UNDEFINED && month_ != current_month) {
      // "month" will be changed

      // figure out the correct year (may be current_year or +1/-1)
      int target_year = current_year;
      if (get_previous && month_ > current_month)
        target_year--;
      if (get_next && month_ < current_month)
        target_year++;

      if (get_previous)
        newtime.Set(target_year, month_,
                    CalendarUtil::MaxDaysInMonth(target_year, month_),
                    23, 59, 59);
      else
        newtime.Set(target_year, month_, 1, 0, 0, 0);
      continue;
    }

    //
    // adjust "week" according to instructions
    //

    if (week_in_year_ != UNDEFINED) {
      ASSERT(month_ == UNDEFINED);

      // calculate week numbering (1 through 53)
      int current_week =
        (CalendarUtil::GetDayOfYear(current_year,
                                    current_month,
                                    current_day) - 1) / 7 + 1;

      if (week_in_year_ != current_week) {
        // "week" needs to be changed

        ASSERT(week_in_year_ >= 1 && week_in_year_ <= 53);

        // figure out the correct year (may be current_year or +1/-1)
        int target_year = current_year;
        if (get_previous && week_in_year_ > current_week)
          target_year--;
        if (get_next && week_in_year_ < current_week)
          target_year++;

        int new_month, new_day;
        if (get_previous) {
          // translate new week number to "month and day" representation
          int n = week_in_year_ * 7;  // last day of that week
          if (n > CalendarUtil::TotalDaysInYear(target_year))
            n = CalendarUtil::TotalDaysInYear(target_year);
          CalendarUtil::FindByDayNum(target_year, n, &new_month, &new_day);
          newtime.Set(target_year, new_month, new_day, 23, 59, 59);
        }
        else {
          int n = week_in_year_ * 7 - 6;  // first day of that week
          CalendarUtil::FindByDayNum(target_year, n, &new_month, &new_day);
          newtime.Set(target_year, new_month, new_day, 0, 0, 0);
        }
        continue;
      }
    }

    //
    // adjust "day" according to instructions (3 types)
    //

    if (week_in_month_ != UNDEFINED) {
      //
      // "week count + day count" format:
      //   2nd Monday of the month; last Sunday of the month, etc.
      //

      ASSERT(week_in_year_ == UNDEFINED);
      ASSERT(day_in_week_ != UNDEFINED);

      int candidate_this_month = LocateByWeekAndDay(current_year,
                                                    current_month,
                                                    week_in_month_,
                                                    day_in_week_);
      if (candidate_this_month != current_day) {
        if (get_previous) {
          if (candidate_this_month < current_day) {
            // go to the candidate date of this month
            newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                        candidate_this_month,
                        23, 59, 59);
          }
          else {
            // go to the last day of the previous month and try again
            newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                        0, 23, 59, 59);
          }
        }
        else {  // get_next
          if (candidate_this_month > current_day) {
            // go to the candidate date of this month
            newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                        candidate_this_month,
                        0, 0, 0);
          }
          else {
            // go to the first day of the next month and try again
            newtime.Set(LocalTime::NO_CHANGE, current_month + 1, 1,
                        0, 0, 0);
          }
        }
        continue;
      }
    }
    else if (day_in_month_ != UNDEFINED) {
      //
      // "day of the month format":  1st, 29th, etc.
      //

      ASSERT(day_in_week_ == UNDEFINED);
      if (day_in_month_ != current_day) {
        if (get_previous) {
          if (day_in_month_ > current_day) {
            // go to the last day of the previous month and try again
            newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                        0, 23, 59, 59);
          }
          else {
            newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                        day_in_month_, 23, 59, 59);
          }
        }
        else {  // get_next
          if (day_in_month_ < current_day) {
            // go to the first day of the next month and try again
            newtime.Set(LocalTime::NO_CHANGE, current_month + 1, 1,
                        0, 0, 0);
          }
          else {
            newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                        day_in_month_, 0, 0, 0);
          }
        }
        continue;
      }
    }
    else if (day_in_week_ != UNDEFINED) {
      //
      // "day of the week format":  Sunday, Friday, etc.
      //

      ASSERT(day_in_month_ == UNDEFINED);
      int d = CalendarUtil::GetDayOfWeek(current_year, current_month,
                                         current_day);
      if (day_in_week_ != d) {
        if (get_previous) {
          int delta =
            (day_in_week_ < d ? (d - day_in_week_) : (d + 7 - day_in_week_));
          newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                      current_day - delta,
                      23, 59, 59);
        }
        else {
          int delta =
            (day_in_week_ > d ? (day_in_week_ - d) : (day_in_week_ + 7 - d));
          newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                      current_day + delta,
                      0, 0, 0);
        }
        continue;
      }
    }

    //
    // adjust "hour" according to instructions
    //

    if (hour_ != UNDEFINED && hour_ != current_hour) {
      int target_day = current_day;
      if (get_previous && hour_ > current_hour)
        target_day--;
      if (get_next && hour_ < current_hour)
        target_day++;
      if (get_previous)
        newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE, target_day,
                    hour_, 59, 59);
      else
        newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE, target_day,
                    hour_, 0, 0);
      continue;
    }

    //
    // adjust "minute" according to instructions
    //

    if (minute_ != UNDEFINED && minute_ != current_minute) {
      int target_hour = current_hour;
      if (get_previous && minute_ > current_minute)
        target_hour--;
      if (get_next && minute_ < current_minute)
        target_hour++;
      if (get_previous)
        newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                    LocalTime::NO_CHANGE,
                    target_hour, minute_, 59);
      else
        newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                    LocalTime::NO_CHANGE,
                    target_hour, minute_, 0);
      continue;
    }

    //
    // adjust "second" according to instructions
    //

    if (second_ != UNDEFINED && second_ != current_second) {
      int target_minute = current_minute;
      if (get_previous && second_ > current_second)
        target_minute--;
      if (get_next && second_ < current_second)
        target_minute++;
      newtime.Set(LocalTime::NO_CHANGE, LocalTime::NO_CHANGE,
                  LocalTime::NO_CHANGE,
                  LocalTime::NO_CHANGE, target_minute, second_);
      continue;
    }

    // If control reaches here, nothing got changed in this round,
    // which means we have already found the target time.
    // We can exit now.
    done = true;
  }

  if (!done)
    newtime = (get_next ?
               LocalTime::InfiniteFuture() :
               LocalTime::InfinitePast());

  if (modified_time != NULL)
    *modified_time = newtime;

  return done;
}


// answer questions such as "when is the second Tuesday of this month?"
// for a given year, month, week_in_month (1 to 5 for forward counting,
// -1 to -5 for reverse couting) and day_in_week
int SharpPoint::LocateByWeekAndDay(int year, int month,
                                   int week_in_month,
                                   int day_in_week) const {
  int last_day = CalendarUtil::MaxDaysInMonth(year, month);
  if (week_in_month > 0) {
    ASSERT(week_in_month >= 1 && week_in_month <= 5);

    // find what day is the 1st of this month
    int day_of_week_on_1st = CalendarUtil::GetDayOfWeek(year, month, 1);
    // find the first day the target weekday occurs
    int first_appearance = 1 + (day_in_week >= day_of_week_on_1st ?
                                (day_in_week - day_of_week_on_1st) :
                                (day_in_week + 7 - day_of_week_on_1st));
    int candidate = first_appearance + 7 * (week_in_month_ - 1);
    if (candidate > last_day) {
      // the target day cannot be found (caller must have asked for the
      // 5th something of the month) -- use the last day
      ASSERT(week_in_month == 5);
      candidate = last_day;
    }
    return candidate;
  }
  else {
    ASSERT(week_in_month <= -1 && week_in_month >= -5);

    // find what day is the last of this month
    int day_of_week_on_last = CalendarUtil::GetDayOfWeek(year, month,
                                                         last_day);
    // find the last day the target weekday occurs
    int last_appearance =
      last_day - (day_in_week <= day_of_week_on_last ?
                  (day_of_week_on_last - day_in_week) :
                  (day_of_week_on_last + 7 - day_in_week));
    int candidate = last_appearance - 7 * (-week_in_month - 1);
    if (candidate < 1) {
      // the target day cannot be found (caller must have asked for the
      // last 5th something of the month) -- use the first day
      ASSERT(week_in_month == -5);
      candidate = 1;
    }
    return candidate;
  }
}

