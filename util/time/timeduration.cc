#include "util/time/timeduration.h"

// initialize from a string
// (return true if successful; upon return, endptr points to the first
//  unparsed character)
bool TimeDuration::Init(const char *start, const char **endptr,
                        string *error_msg) {
  *endptr = strutil::SkipSpaces(start);

  error_msg->clear();

  *endptr = ParseNext(*endptr, 'y', &years_, error_msg);
  if (!error_msg->empty()) return false;

  *endptr = ParseNext(*endptr, 'M', &months_, error_msg);
  if (!error_msg->empty()) return false;

  int num_weeks = 0;
  *endptr = ParseNext(*endptr, 'w', &num_weeks, error_msg);
  if (!error_msg->empty()) return false;

  int num_days = 0;
  *endptr = ParseNext(*endptr, 'd', &num_days, error_msg);
  if (!error_msg->empty()) return false;

  days_ = num_weeks * 7 + num_days;

  *endptr = ParseNext(*endptr, 'h', &hours_, error_msg);
  if (!error_msg->empty()) return false;

  *endptr = ParseNext(*endptr, 'm', &minutes_, error_msg);
  if (!error_msg->empty()) return false;

  *endptr = ParseNext(*endptr, 's', &seconds_, error_msg);
  if (!error_msg->empty()) return false;

  VLOG(5) << "Parsed \"" << string(start, *endptr - start)
         << "\" as: " << Print();

  return true;
}


// for debug: return human-redable string describing this object
string TimeDuration::Print() const {
  if (GetSign() == 0)
    return "--- zero duration ---";
  else {
    stringstream ss;
    PrintAppend(years_, "year", ss);
    PrintAppend(months_, "month", ss);
    PrintAppend(days_, "day", ss);
    PrintAppend(hours_, "hour", ss);
    PrintAppend(minutes_, "minute", ss);
    PrintAppend(seconds_, "second", ss);
    return ss.str();
  }
}

string TimeDuration::PrintShort() const {
  if (GetSign() == 0)
    return "s0";
  else {
    stringstream ss;
    if (years_ != 0) ss << "y" << years_;
    if (months_ != 0) ss << "M" << months_;
    if (days_ != 0) ss << "d" << days_;
    if (hours_ != 0) ss << "h" << hours_;
    if (minutes_ != 0) ss << "m" << minutes_;
    if (seconds_ != 0) ss << "s" << seconds_;
    return ss.str();
  }
}

// helper function for Print()
void TimeDuration::PrintAppend(int num, const string& name,
                               stringstream& ss) const {
  if (num != 0) {
    ss << num << " " << name;
    if (num > 1 || num < -1)
      ss << "s";
    ss << " ";
  }
}

