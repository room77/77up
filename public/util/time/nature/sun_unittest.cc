#include "base/common.h"
#include "util/time/nature/sun.h"

FLAG_int(year, 2008, "test year");
FLAG_int(month, 2, "test month");
FLAG_int(day, 9, "test day");
FLAG_int(hour, 12, "test hour");
FLAG_int(minute, 30, "test minute");
FLAG_int(second, 0, "test second");

void TestSun(Time t, const Place& place, bool is_default) {
  LOG(INFO) << "Time: " << place.PrintTime(t);

  Time prev_boundary, next_boundary;
  double altitude;

  bool is_daytime =
    Nature::IsDayTime(t, place, &prev_boundary, &next_boundary, &altitude);

  LOG(INFO) << (is_daytime ? "is day time" : "is night time");
  LOG(INFO) << "Previous boundary: " << prev_boundary.Print(place.get_tz());
  LOG(INFO) << "Next boundary: " << next_boundary.Print(place.get_tz());

  if (is_default) {
    // in San Francisco, we always have sunrise/sunset
    ASSERT(prev_boundary.IsFinite());
    ASSERT(next_boundary.IsFinite());

    LocalTime prev_local = prev_boundary.GetLocalTime(place.get_tz());
    LocalTime next_local = next_boundary.GetLocalTime(place.get_tz());

    // sunrise in San Francsico is always before noon
    // sunset in San Francsico is always after noon
    if (is_daytime) {
      ASSERT(prev_local.get_hour() < 12);
      ASSERT(next_local.get_hour() > 12);
    }
    else {
      ASSERT(prev_local.get_hour() > 12);
      ASSERT(next_local.get_hour() < 12);
    }
  }

  ASSERT(prev_boundary <= t);
  ASSERT(next_boundary >= t);

  LOG(INFO) << "-----";
  Time prev_sunrise = Nature::PreviousSunrise(t, place);
  Time prev_sunset  = Nature::PreviousSunset(t, place);
  Time next_sunrise = Nature::NextSunrise(t, place);
  Time next_sunset  = Nature::NextSunset(t, place);
  LOG(INFO) << "Previous sunrise: " << place.PrintTime(prev_sunrise);
  LOG(INFO) << "Previous sunset: " << place.PrintTime(prev_sunset);
  LOG(INFO) << "Next sunrise: " << place.PrintTime(next_sunrise);
  LOG(INFO) << "Next sunset: " << place.PrintTime(next_sunset);
  if (is_daytime) {
    ASSERT(prev_boundary == prev_sunrise);
    ASSERT(next_boundary == next_sunset);
    ASSERT(prev_sunset <= prev_sunrise);
    ASSERT(next_sunrise >= next_sunset);
  }
  else {
    ASSERT(prev_boundary == prev_sunset);
    ASSERT(next_boundary == next_sunrise);
    ASSERT(prev_sunrise <= prev_sunset);
    ASSERT(next_sunset >= next_sunrise);
  }
}

void TestPole() {
  Timezone tz = Timezone::PST();  // US Pacific Time
  LatLong northpole_latlong = LatLong::Create(89, -122);
  LatLong southpole_latlong = LatLong::Create(-89, -122);

  Place northpole(northpole_latlong, tz);
  Place southpole(southpole_latlong, tz);

  Time feb(2008, 2, 9, 12, 30, 0, tz);
  Time may(2008, 5, 9, 12, 30, 0, tz);

  Time prev_boundary, next_boundary;
  double altitude;

  // perpetual daylight

  ASSERT(Nature::IsDayTime(feb, southpole, &prev_boundary, &next_boundary,
                           &altitude));
  ASSERT(!prev_boundary.IsFinite());
  ASSERT(!next_boundary.IsFinite());
  ASSERT(altitude > 0);

  ASSERT(Nature::IsDayTime(may, northpole, &prev_boundary, &next_boundary,
                           &altitude));
  ASSERT(!prev_boundary.IsFinite());
  ASSERT(!next_boundary.IsFinite());
  ASSERT(altitude > 0);

  // perpetual night

  ASSERT(!Nature::IsDayTime(feb, northpole, &prev_boundary, &next_boundary,
                            &altitude));
  ASSERT(!prev_boundary.IsFinite());
  ASSERT(!next_boundary.IsFinite());
  ASSERT(altitude < 0);

  ASSERT(!Nature::IsDayTime(may, southpole, &prev_boundary, &next_boundary,
                            &altitude));
  ASSERT(!prev_boundary.IsFinite());
  ASSERT(!next_boundary.IsFinite());
  ASSERT(altitude < 0);
}

int init_main() {
  Timezone tz = Timezone::PST();  // US Pacific Time

  LatLong p1 = LatLong::Create(37, -122);
  Time t1(2008, 2, 9, 12, 30, 0, tz);
  Time t2(2008, 6, 1, 1, 30, 0, tz);
  LOG(INFO) << "***** Testing default time (1) *****";
  TestSun(t1, Place(p1, tz), true);
  LOG(INFO) << "***** Testing default time (2) *****";
  TestSun(t2, Place(p1, tz), true);

  TestPole();  // test north pole / south pole

  LOG(INFO) << "--------------------------------------------";
  LatLong p2 = LatLong::Create(37, -122);
  Time t3(gFlag_year, gFlag_month, gFlag_day,
          gFlag_hour, gFlag_minute, gFlag_second, tz);
  LOG(INFO) << "Testing time supplied by command-line:";
  TestSun(t3, Place(p2, tz), false);

  LOG(INFO) << "PASS";
  return 0;
}
