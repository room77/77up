#include "base/common.h"
#include "util/time/utime.h"
#include "util/time/timeutil.h"

FLAG_int(year, 2008, "test year");
FLAG_int(month, 4, "test month");
FLAG_int(day, 10, "test day");
FLAG_int(hour, 12, "test hour");
FLAG_int(minute, 30, "test minute");
FLAG_int(second, 0, "test second");

void TestCalendarUtil() {
  ASSERT(CalendarUtil::IsLeapYear(2008));
  ASSERT(CalendarUtil::IsLeapYear(2000));
  ASSERT(!CalendarUtil::IsLeapYear(2100));
  ASSERT(!CalendarUtil::IsLeapYear(2009));

  ASSERT_EQ(CalendarUtil::TotalDaysInYear(2009), 365);
  ASSERT_EQ(CalendarUtil::TotalDaysInYear(2012), 366);
  ASSERT_EQ(CalendarUtil::TotalDaysInYear(2200), 365);
  ASSERT_EQ(CalendarUtil::TotalDaysInYear(2400), 366);

  ASSERT_EQ(CalendarUtil::MaxDaysInMonth(2000, 2), 29);
  ASSERT_EQ(CalendarUtil::MaxDaysInMonth(2000, 11), 30);
  ASSERT_EQ(CalendarUtil::MaxDaysInMonth(2007, 12), 31);
  ASSERT_EQ(CalendarUtil::MaxDaysInMonth(2007, 6), 30);

  ASSERT_EQ(CalendarUtil::GetDayOfYear(2007, 1, 1), 1);
  ASSERT_EQ(CalendarUtil::GetDayOfYear(2007, 12, 31), 365);
  ASSERT_EQ(CalendarUtil::GetDayOfYear(2008, 1, 1), 1);
  ASSERT_EQ(CalendarUtil::GetDayOfYear(2008, 12, 31), 366);
  ASSERT_EQ(CalendarUtil::GetDayOfYear(2008, 2, 29), 60);
  ASSERT_EQ(CalendarUtil::GetDayOfYear(2008, 3, 1), 61);
  ASSERT_EQ(CalendarUtil::GetDayOfYear(2007, 3, 1), 60);

  int month, day;
  CalendarUtil::FindByDayNum(2009, 60, &month, &day);
  ASSERT(month == 3 && day == 1);
  CalendarUtil::FindByDayNum(2008, 366, &month, &day);
  ASSERT(month == 12 && day == 31);
  CalendarUtil::FindByDayNum(2008, 60, &month, &day);
  ASSERT(month == 2 && day == 29);
  CalendarUtil::FindByDayNum(2009, 365, &month, &day);
  ASSERT(month == 12 && day == 31);

  ASSERT_EQ(CalendarUtil::DaysSince1970(2008, 4, 1), 13970);
  int yy, mm, dd;
  CalendarUtil::ElapseFrom1970(13970, &yy, &mm, &dd);
  ASSERT(yy == 2008 && mm == 4 && dd == 1);

  ASSERT_EQ(CalendarUtil::DaysSince1970(2008, 1, 1), 13879);
  CalendarUtil::ElapseFrom1970(13879, &yy, &mm, &dd);
  ASSERT(yy == 2008 && mm == 1 && dd == 1);

  ASSERT_EQ(CalendarUtil::DaysSince1970(2007, 12, 31), 13878);
  CalendarUtil::ElapseFrom1970(13878, &yy, &mm, &dd);
  ASSERT(yy == 2007 && mm == 12 && dd == 31);

  ASSERT_EQ(CalendarUtil::DaysSince1970(2007, 12, 29), 13876);
  CalendarUtil::ElapseFrom1970(13876, &yy, &mm, &dd);
  ASSERT(yy == 2007 && mm == 12 && dd == 29);

  // April 1, 2008 is a Tuesday
  ASSERT_EQ(CalendarUtil::GetDayOfWeek(2008, 4, 1), 3);

  // February 29, 2008 is a Friday
  ASSERT_EQ(CalendarUtil::GetDayOfWeek(2008, 2, 29), 6);

  // January 1, 1990 is a Monday
  ASSERT_EQ(CalendarUtil::GetDayOfWeek(1990, 1, 1), 2);

  // December 29, 2007 is a Saturday
  ASSERT_EQ(CalendarUtil::GetDayOfWeek(2007, 12, 29), 7);

  // January 5, 2200 is a Sunday
  ASSERT_EQ(CalendarUtil::GetDayOfWeek(2200, 1, 5), 1);

}

void TestTime_Auto() {
  Timezone tz = Timezone::PST();

  Time now;
  LOG(INFO) << "Now: " << now.Print(tz) << " PST";
  LOG(INFO) << "--------------------------------------------";
  static_assert(sizeof(Time) == sizeof(time_t),
                "Time and time_t have different sizes.");

  Time t(2008, 4, 10, 12, 30, 0, tz);
  ASSERT_EQ(t.Print(tz), "4/10/2008 12:30:00");
  ASSERT_EQ(t.LastMidnight(tz).Print(tz), "4/10/2008 00:00:00");
  ASSERT_EQ(t.NextMidnight(tz).Print(tz), "4/11/2008 00:00:00");
  ASSERT_EQ(t.LastFullHour(tz).Print(tz), "4/10/2008 12:00:00");
  ASSERT_EQ(t.NextFullHour(tz).Print(tz), "4/10/2008 13:00:00");
  ASSERT_EQ(t.LastMultiple(30 * ONE_MINUTE).Print(tz), "4/10/2008 12:30:00");
  ASSERT_EQ(t.NextMultiple(30 * ONE_MINUTE).Print(tz), "4/10/2008 13:00:00");
  ASSERT_EQ(t.CushionForward(45 * ONE_MINUTE, 30 * ONE_MINUTE).Print(tz),
            "4/10/2008 13:30:00");
  Time prev, next;
  ASSERT(TimeUtil::CheckDomain(TimeDomain<SharpPoint>("[(h9)(h17)]"), t, tz,
                               &prev, &next));
  ASSERT_EQ(prev.Print(tz), "4/10/2008 09:00:00");
  ASSERT_EQ(next.Print(tz), "4/10/2008 17:00:00");

  LatLong p = LatLong::Create(37, -122);
  Place place(p, tz);
  ASSERT(TimeUtil::CheckDomain(TimeDomain<GenericPoint>("[(z1)(z2)]"),
                               t, place, &prev, &next));

  // test around DST change
  Time t1(2008, 3, 9, 1, 30, 0, tz);
  ASSERT_EQ(t1.LastMidnight(tz).Print(tz), "3/9/2008 00:00:00");
  ASSERT_EQ(t1.NextMidnight(tz).Print(tz), "3/10/2008 00:00:00");
  ASSERT_EQ(t1.LastFullHour(tz).Print(tz), "3/9/2008 01:00:00");
  ASSERT_EQ(t1.NextFullHour(tz).Print(tz), "3/9/2008 03:00:00");
  ASSERT_EQ(t1.LastMultiple(30 * ONE_MINUTE).Print(tz), "3/9/2008 01:30:00");
  ASSERT_EQ(t1.NextMultiple(30 * ONE_MINUTE).Print(tz), "3/9/2008 03:00:00");

  Time t2(2008, 11, 2, 1, 30, 0, tz, 1);  // 1:30am DST
  ASSERT_EQ(t2.LastMidnight(tz).Print(tz), "11/2/2008 00:00:00");
  ASSERT_EQ(t2.NextMidnight(tz).Print(tz), "11/3/2008 00:00:00");
  ASSERT_EQ(t2.LastFullHour(tz).Print(tz), "11/2/2008 01:00:00");
  ASSERT_EQ(t2.NextFullHour(tz).Print(tz), "11/2/2008 01:00:00");
  ASSERT_EQ(t2.LastMultiple(30 * ONE_MINUTE).Print(tz), "11/2/2008 01:30:00");
  ASSERT_EQ(t2.NextMultiple(30 * ONE_MINUTE).Print(tz), "11/2/2008 01:00:00");

  Time t3(2008, 11, 2, 1, 30, 0, tz);  // 1:30am standard time
  ASSERT_EQ(t3.LastMidnight(tz).Print(tz), "11/2/2008 00:00:00");
  ASSERT_EQ(t3.NextMidnight(tz).Print(tz), "11/3/2008 00:00:00");
  ASSERT_EQ(t3.LastFullHour(tz).Print(tz), "11/2/2008 01:00:00");
  ASSERT_EQ(t3.NextFullHour(tz).Print(tz), "11/2/2008 02:00:00");
  ASSERT_EQ(t3.LastMultiple(30 * ONE_MINUTE).Print(tz), "11/2/2008 01:30:00");
  ASSERT_EQ(t3.NextMultiple(30 * ONE_MINUTE).Print(tz), "11/2/2008 02:00:00");
}

void TestTime_Manual() {
  Timezone tz = Timezone::PST();
  Time t(gFlag_year, gFlag_month, gFlag_day,
         gFlag_hour, gFlag_minute, gFlag_second, tz);

  LOG(INFO) << "Time: " << t.Print(tz)
         << " (date: " << t.PrintDateOnly(tz) << ")";

  LOG(INFO) << "Last midnight: " << t.LastMidnight(tz).Print(tz);
  LOG(INFO) << "Next midnight: " << t.NextMidnight(tz).Print(tz);

  LOG(INFO) << "Last full hour: " << t.LastFullHour(tz).Print(tz);
  LOG(INFO) << "Next full hour: " << t.NextFullHour(tz).Print(tz);

  LOG(INFO) << "Last multiple of 30 minutes: "
         << t.LastMultiple(30 * ONE_MINUTE).Print(tz);
  LOG(INFO) << "Next multiple of 30 minutes: "
         << t.NextMultiple(30 * ONE_MINUTE).Print(tz);
  LOG(INFO) << "Next multiple of 30 minutes with 45-minute cushion: "
         << t.CushionForward(45 * ONE_MINUTE, 30 * ONE_MINUTE).Print(tz);

  LOG(INFO) << "Checking against time-domain 9am-5pm:";
  Time prev, next;
  bool exists = TimeUtil::CheckDomain(TimeDomain<SharpPoint>("[(h9)(h17)]"),
                                      t, tz,
                                      &prev, &next);
  LOG(INFO) << (exists ? "in domain" : "not in domain");
  LOG(INFO) << "Previous boundary: " << prev.Print(tz);
  LOG(INFO) << "Next boundary: " << next.Print(tz);
}

int init_main() {

  // test CalendarUtil functions
  TestCalendarUtil();

  // test Time class
  TestTime_Auto();
  TestTime_Manual();  // test from command-line input

  LOG(INFO) << "PASS";
  return 0;
}
