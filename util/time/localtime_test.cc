// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/time/localtime.h"

#include <chrono>

#include "test/cc/test_main.h"

namespace test {

typedef std::chrono::duration<int, std::ratio<86400>> integer_day;

TEST(LocalDate, ParseSanity) {
  LocalDate d = LocalDate::ParseFromYYYYMMDD("20110801");
  EXPECT_EQ(2011, d.get_year());
  EXPECT_EQ(8, d.get_month());
  EXPECT_EQ(1, d.get_day());

  d = LocalDate::ParseFromYYYYMMDD("2011-08-01", '-');
  EXPECT_EQ(2011, d.get_year());
  EXPECT_EQ(8, d.get_month());
  EXPECT_EQ(1, d.get_day());
}

TEST(LocalDate, PrintFormatted) {
  LocalDate d(2012, 10, 22);
  EXPECT_EQ("2012-10-22", d.PrintFormatted("%Y-%m-%d"));
  EXPECT_EQ("2012/10/22", d.PrintFormatted("%Y/%m/%d"));
  EXPECT_EQ("12/10/22", d.PrintFormatted("%y/%m/%d"));
  EXPECT_EQ("1022", d.PrintFormatted("%m%d"));
  EXPECT_EQ("Oct-OCT-10", d.PrintFormatted("%b-%B-%m"));
  EXPECT_EQ("Mon-MON-22", d.PrintFormatted("%a-%A-%d"));
}

TEST(LocalDate, SingleDigitDates) {
  LocalDate d(2012, 1, 1);
  EXPECT_EQ("2012-01-01", d.PrintFormatted("%Y-%m-%d"));
}

TEST(LocalDate, PrintingOptions) {
  LocalDate d(2012, 10, 22);
  EXPECT_EQ("2012-10-22", d.PrintDate_YYYYMMDD("-"));
  EXPECT_EQ("2012/10/22", d.PrintDate_YYYYMMDD("/"));

  EXPECT_EQ("10-22-2012", d.PrintDate_MMDDYYYY("-"));
  EXPECT_EQ("10/22/2012", d.PrintDate_MMDDYYYY("/"));

  EXPECT_EQ("22-10-2012", d.PrintDate_DDMMYYYY("-"));
  EXPECT_EQ("22/10/2012", d.PrintDate_DDMMYYYY("/"));

  EXPECT_EQ("22OCT", d.PrintDate_DDMMM());
  EXPECT_EQ("22OCT12", d.PrintDate_DDMMMYY());

  EXPECT_EQ("10/22", d.PrintDate_M_D());
  EXPECT_EQ("Oct 22", d.PrintDate_Mname_D());
  EXPECT_EQ("Mon 10/22", d.PrintDate_DW_M_D());
  EXPECT_EQ("Mon 10/22/2012", d.PrintDate_DW_M_D_Y());
}

TEST(LocalDate, OutputToday) {
  LocalDate today = LocalDate::Today();
  LOG(INFO) << today.PrintDate_YYYYMMDD("-");
}

TEST(LocalTime, Sanity) {
  LocalTime t(2008, 4, 8, 19, 15, 0);
  EXPECT_EQ(1207682100, t.SecondsSince1970());
  t.ElapseFrom1970(1207768500);
  EXPECT_EQ("4/9/2008 19:15:00", t.Print());
}

TEST(LocalTime, UTCTimeFromDuration) {
  {
    LocalTime t = LocalTime::UTCTimeFromDuration(
        chrono::duration_cast<integer_day>(chrono::seconds(1381178333)));
    EXPECT_EQ(15985, t.DaysSince1970());
    EXPECT_EQ("10/7/2013 00:00:00", t.Print());
  }
  {
    LocalTime t = LocalTime::UTCTimeFromDuration(
        chrono::duration_cast<chrono::hours>(chrono::seconds(1381178333)));
    EXPECT_EQ(15985, t.DaysSince1970());
    EXPECT_EQ(1381176000, t.SecondsSince1970());
    EXPECT_EQ("10/7/2013 20:00:00", t.Print());
  }
  {
    LocalTime t = LocalTime::UTCTimeFromDuration(
        chrono::duration_cast<chrono::minutes>(chrono::seconds(1381178333)));
    EXPECT_EQ(15985, t.DaysSince1970());
    EXPECT_EQ(1381178280, t.SecondsSince1970());
    EXPECT_EQ("10/7/2013 20:38:00", t.Print());
  }
  {
    LocalTime t = LocalTime::UTCTimeFromDuration(chrono::seconds(1381178333));
    EXPECT_EQ(15985, t.DaysSince1970());
    EXPECT_EQ(1381178333, t.SecondsSince1970());
    EXPECT_EQ("10/7/2013 20:38:53", t.Print());
  }
  {
    LocalTime t = LocalTime::UTCTimeFromDuration(
        chrono::duration_cast<chrono::milliseconds>(chrono::seconds(1381178333)));
    EXPECT_EQ(15985, t.DaysSince1970());
    EXPECT_EQ(1381178333, t.SecondsSince1970());
    EXPECT_EQ("10/7/2013 20:38:53", t.Print());
  }
  {
    LocalTime t = LocalTime::UTCTimeFromDuration(
        chrono::duration_cast<chrono::microseconds>(chrono::seconds(1381178333)));
    EXPECT_EQ(15985, t.DaysSince1970());
    EXPECT_EQ(1381178333, t.SecondsSince1970());
    EXPECT_EQ("10/7/2013 20:38:53", t.Print());
  }
}

TEST(LocalTime, UTCTimeFromTimeStamp) {
  LocalTime t = LocalTime::UTCTimeFromTimeStamp(
      chrono::duration_cast<chrono::microseconds>(chrono::seconds(1381178333)).count());
  EXPECT_EQ(1381178333, t.SecondsSince1970());
  EXPECT_EQ("10/7/2013 20:38:53", t.Print());
}

}  // namespace test
