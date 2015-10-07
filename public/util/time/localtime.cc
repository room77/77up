#include "util/time/localtime.h"

// normalize all fields so that all numbers are within range
void LocalDate::NormalizeDate() {
  // make infinite past if specified before 1970
  if (y_ < 1970) {
    *this = InfinitePast();
    return;
  }

  // normalize "month", carry to "year"
  //   (needs to be done before "day" because
  //    "day" normalization depends on knowing the month)
  int mon = m_ - 1;  // change month range to [0, 11]
  NormalizeField(12, mon, y_);
  m_ = mon + 1;      // reset month range to [1, 12]

  // finally, normalize "day"
  if (d_ > -20 && d_ < 50) {
    // simple case: day is not off by too much
    // -- use a faster method of normalization
    if (d_ < 1) {
      --m_;
      if (m_ < 1) {
        // need further adjustment for "year" and "month"
        --y_;
        m_ = 12;
      }
      d_ += CalendarUtil::MaxDaysInMonth(y_, m_);
      ASSERT(d_ >= 1);
    }
    else {
      int num_days = CalendarUtil::MaxDaysInMonth(y_, m_);
      if (d_ > num_days) {
        ++m_;
        if (m_ > 12) {
          // need further adjustment for "year" and "month"
          ++y_;
          m_ = 1;
        }
        d_ -= num_days;
        ASSERT_LE(d_, 28);
      }
    }
  }
  else {
    // complex case -- use generic method to normalize "day" (a bit slower)

    // first, find how many days have elapsed from 1/1/1970 to
    // the 1st of this month
    int seq_1st = CalendarUtil::DaysSince1970(y_, m_, 1);
    // next, calculate how may days have elapsed from 1/1/1970 to the given day
    int seq = seq_1st + (d_ - 1);

    // convert the sequence number back to year/month/day
    CalendarUtil::ElapseFrom1970(seq, &y_, &m_, &d_);
  }
}


LocalDate LocalDate::ParseFromYYYYMMDD(const string& s, const char delimiter) {
  int d;
  if (delimiter) {
    ASSERT(s.size() == 10);

    std::stringstream ss;
    for(int i=0; i < s.size(); i++) {
      if (s[i] != delimiter) {
        ss << s[i];
      }
    }
    d = atoi(ss.str().c_str());
  }
  else {
    ASSERT(s.size() == 8);
    d = atoi(s.c_str());
  }
  LocalDate local(d / 10000, (d % 10000) / 100, d % 100);
  return local;
}

LocalDate LocalDate::ParseFromMMDDYYYY(const string& s, const char delimiter) {
  int d;
  if (delimiter) {
    ASSERT(s.size() == 10);
    std::stringstream ss;
    for (int i = 0; i < s.size(); i++) {
      if (s[i] != delimiter) {
        ss << s[i];
      }
    }
    d = atoi(ss.str().c_str());
  } else {
    ASSERT(s.size() == 8);
    d = atoi(s.c_str());
  }
  LocalDate local(d % 10000, d / 1000000, (d % 1000000) / 10000);
  return local;
}

LocalDate LocalDate::Today() {
  time_t t = time(0);
  struct tm *now = localtime(&t);
  LocalDate local(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
  return local;
}

string LocalDate::DayOfWeek_EnglishAbbr(int index) {
  const char *wd_name[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  ASSERT(index >= 0 && index <= 6);
  return wd_name[index];
}

string LocalDate::Month_EnglishAbbr(int index) {
  const char *m_name[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  ASSERT(index >= 0 && index <= 11);
  return m_name[index];
}

bool LocalDate::PrintSpecifier(char specifier, ostream& out) const {
  bool res = true;
  switch(specifier) {
    case 'a' : out << GetDayOfWeek_Abbr(); break;
    case 'A' : out << strutil::ToUpper(GetDayOfWeek_Abbr()); break;
    case 'b' : out << GetMonth_Abbr(); break;
    case 'B' : out << strutil::ToUpper(GetMonth_Abbr()); break;
    case 'd' : out << setw(2) << setfill('0') << d_; break;
    case 'm' : out << setw(2) << setfill('0') << m_; break;
    case 'y' : out << setw(2) << setfill('0') << (y_ % 100); break;
    case 'Y' : out << setw(4) << setfill('0') << y_; break;
    default: {
      res = false;
      break;
    }
  }
  return res;
}

string LocalDate::PrintFormatted(const string& format) const {
  if (IsInfinitePast()) return "Infinite Past";
  else if (IsInfiniteFuture()) return "Infinite Future";
  else if (!IsFinite()) return "";

  std::stringstream res;
  int index = 0;
  while(index < format.size()) {
    const char val = format[index++];
    if (val != '%') {
      res << val;
      continue;
    }

    if(index >= format.size()) {
      VLOG(2) << "Invalid Format string. Expected Specifier after %" << format;
      return "";
    }
    const char specifier =  format[index++];
    ASSERT(PrintSpecifier(specifier, res)) << "Unknown specifier: %" << specifier;
  }

  return res.str();
}

// convert from/to unix "struct tm" format
void LocalTime::FromTM(const struct tm *t) {
  y_ = t->tm_year + 1900;
  m_ = t->tm_mon + 1;
  d_ = t->tm_mday;
  h_ = t->tm_hour;
  mn_ = t->tm_min;
  s_ = t->tm_sec;
  is_dst_ = -1;
  Normalize();
}

void LocalTime::ToTM(struct tm *t) const {
  t->tm_year = y_ - 1900;
  t->tm_mon = m_ - 1;
  t->tm_mday = d_;
  t->tm_hour = h_;
  t->tm_min = mn_;
  t->tm_sec = s_;
  t->tm_wday = GetDayOfWeek() - 1;
  t->tm_yday = GetDayOfYear() - 1;
  t->tm_isdst = -1;
}

LocalTime LocalTime::ParseFromYYYYMMDDHHMM(const string& yyyymmdd,
                                           const string& hhmm) {
  ASSERT(yyyymmdd.size() == 8);
  ASSERT(hhmm.size() <= 4);

  int d = atoi(yyyymmdd.c_str());
  int t = atoi(hhmm.c_str());
  LocalTime local(d / 10000, (d % 10000) / 100, d % 100,
                  t / 100, t % 100, 0);
  return local;
}

// normalize all fields so that all numbers are within range
void LocalTime::Normalize() {
  // normalize "second", carry to "minute"
  NormalizeField(60, s_, mn_);

  // normalize "minute", carry to "hour"
  NormalizeField(60, mn_, h_);

  // normalize "hour", carry to "day"
  NormalizeField(24, h_, d_);

  NormalizeDate();
}
