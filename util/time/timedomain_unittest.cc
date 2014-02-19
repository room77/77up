#include "util/time/sharppoint.h"
#include "util/time/genericpoint.h"
#include "util/time/timedomain.h"

FLAG_int(year, 2008, "test year");
FLAG_int(month, 4, "test month");
FLAG_int(day, 7, "test day");
FLAG_int(hour, 12, "test hour");
FLAG_int(minute, 30, "test minute");
FLAG_int(second, 0, "test second");

FLAG_string(spec, "[(h11)(z2)]", "GDF 4.0 timedomain spec to test");

FLAG_string(pointspec, "", "GenericPoint spec to test");


void TestSharpPoint_Auto(int year, int month, int day,
                         int hour, int minute, int second,
                         const string& constraint, bool get_next,
                         const string& answer) {
  SharpPoint b;
  string error_msg;
  ASSERT(b.Init(constraint, &error_msg))
    << "Error parsing " << constraint << ": " << error_msg;

  LocalTime current(year, month, day, hour, minute, second);
  LocalTime newtime;

  if (get_next)
    b.GetNext(current, NULL, &newtime);
  else
    b.GetPrevious(current, NULL, &newtime);
  string newtime_string = newtime.Print();

  ASSERT(newtime_string == answer)
    << current.Print() << ", " << (get_next ? "next " : "previous ")
    << b.PrintShort()
    << ": / found: " << newtime_string << "; answer given: " << answer;
}

void TestSharpPoint() {
  // automated testing
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "h9", true,
                      "4/3/2008 09:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "h9", false,
                      "4/2/2008 09:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "y2007M1", true,
                      "Infinite Future");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "y2007M1", false,
                      "1/1/2007 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "f13", true,
                      "5/6/2008 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "f14h18", true,
                      "4/2/2008 18:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "f15", true,
                      "4/3/2008 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "f21", true,
                      "4/13/2008 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "l11", true,
                      "4/27/2008 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "l22", false,
                      "3/24/2008 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "l53", false,
                      "4/1/2008 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "d10", true,
                      "4/10/2008 00:00:00");
  TestSharpPoint_Auto(2008, 4, 2, 12, 0, 0, "t5", true,
                      "4/3/2008 00:00:00");
}

void TestSharpDomain_Auto(int year, int month, int day,
                          int hour, int minute, int second,
                          const string& constraint,
                          bool answer_in_domain,
                          const string& answer_prev_boundary,
                          const string& answer_next_boundary) {
  TimeDomain<SharpPoint> td;
  string error_msg;
  ASSERT(td.Init(constraint, &error_msg))
    << "Error parsing " << constraint << ": " << error_msg;

  // test TimeDomain's copy constructor
  TimeDomain<SharpPoint> td2 = td;

  LocalTime current(year, month, day, hour, minute, second);
  LocalTime prev, next;

  bool in_domain = td2.CheckTime(current, NULL, &prev, &next);

  ASSERT(in_domain == answer_in_domain)
    << current.Print() << ", " << td2.PrintShort()
    << ": / found: " << (in_domain ? "in domain" : "not in domain")
    << "; answer given: "
    << (answer_in_domain ? "in domain" : "not in domain");

  string prev_string = prev.Print();
  string next_string = next.Print();

  ASSERT(prev_string == answer_prev_boundary)
    << current.Print() << ", " << td2.PrintShort()
    << ": / prev boundary calculated: " << prev_string
    << "; answer given: " << answer_prev_boundary;
  ASSERT(next_string == answer_next_boundary)
    << current.Print() << ", " << td2.PrintShort()
    << ": / next boundary calculated: " << next_string
    << "; answer given: " << answer_next_boundary;
}

void TestGenericDomain_Auto(int year, int month, int day,
                            int hour, int minute, int second,
                            const Place& place,
                            const string& constraint,
                            bool answer_in_domain,
                            const string& answer_prev_boundary,
                            const string& answer_next_boundary) {
  TimeDomain<GenericPoint> td;
  string error_msg;
  ASSERT(td.Init(constraint, &error_msg))
    << "Error parsing " << constraint << ": " << error_msg;

  // test TimeDomain's copy constructor
  TimeDomain<GenericPoint> td2 = td;

  LocalTime current(year, month, day, hour, minute, second);
  LocalTime prev, next;

  bool in_domain = td2.CheckTime(current, &place, &prev, &next);

  ASSERT(in_domain == answer_in_domain)
    << current.Print() << ", " << td2.PrintShort()
    << ": / found: " << (in_domain ? "in domain" : "not in domain")
    << "; answer given: "
    << (answer_in_domain ? "in domain" : "not in domain");

  string prev_string = prev.Print();
  string next_string = next.Print();

  ASSERT(prev_string == answer_prev_boundary)
    << current.Print() << ", " << td2.PrintShort()
    << ": / prev boundary calculated: " << prev_string
    << "; answer given: " << answer_prev_boundary;
  ASSERT(next_string == answer_next_boundary)
    << current.Print() << ", " << td2.PrintShort()
    << ": / next boundary calculated: " << next_string
    << "; answer given: " << answer_next_boundary;
}

void TestTimeDomain() {
  // automated testing
  TestSharpDomain_Auto(2008, 4, 8, 16, 0, 0, "[(h9)(h17)]",
                       true, "4/8/2008 09:00:00", "4/8/2008 17:00:00");
  TestSharpDomain_Auto(2008, 4, 8, 16, 0, 0, "[(h20)(h22)]",
                       false, "4/7/2008 22:00:00", "4/8/2008 20:00:00");
  TestSharpDomain_Auto(2008, 4, 8, 16, 0, 0, "[(h9)(h17)]*[(t2){d5}]",
                       true, "4/8/2008 09:00:00", "4/8/2008 17:00:00");
  TestSharpDomain_Auto(2008, 4, 12, 16, 0, 0, "[(h9)(h17)]*[(t2){d5}]",
                       false, "4/11/2008 17:00:00", "4/14/2008 09:00:00");
  TestSharpDomain_Auto(2008, 4, 8, 16, 0, 0, "[[(M3f21){M10}]-[(M11f11){M3}]]",
                       true, "3/9/2008 00:00:00", "11/2/2008 00:00:00");
  TestSharpDomain_Auto(2008, 4, 8, 16, 0, 0, "[(M3f21)(M11f11)]",
                       true, "3/9/2008 00:00:00", "11/2/2008 00:00:00");
  TestSharpDomain_Auto(2008, 4, 8, 16, 0, 0,
                       "[[(M3f21-h1){M10}]-[(M11f11+M1d1m90){M3}]]",
                       true, "3/8/2008 23:00:00", "12/3/2008 01:30:00");
  TestSharpDomain_Auto(2008, 4, 8, 16, 0, 0,
                       "[[(h9)(h17)]-[(h12)(h13)]+[(h20)(h22)]] * [(f23)(t4)]",
                       true, "4/8/2008 13:00:00", "4/8/2008 17:00:00");
  TestSharpDomain_Auto(2008, 4, 8, 17, 0, 0,
                       "[[(h9)(h17)]-[(h12)(h13)]+[(h20)(h22)]] * [(f23)(t4)]",
                       false, "4/8/2008 17:00:00", "4/8/2008 20:00:00");
  TestSharpDomain_Auto(2008, 4, 8, 18, 0, 0,
                       "[[(h9)(h17)]-[(h12)(h13)]+[(h20)(h22)]] * [(l43)(t4)]",
                       false, "4/8/2008 17:00:00", "4/8/2008 20:00:00");
  TestSharpDomain_Auto(2008, 4, 9, 16, 0, 0,
                       "[[(h9)(h17)]-[(h12)(h13)]+[(h20)(h22)]] * [(l43)(t4)]",
                       false, "4/8/2008 22:00:00", "5/6/2008 09:00:00");
  TestSharpDomain_Auto(2008, 4, 9, 16, 0, 0, "[(y2008M1d1)(y2008M2d1)]",
                       false, "2/1/2008 00:00:00", "Infinite Future");
  TestSharpDomain_Auto(2008, 4, 9, 16, 0, 0, "[(y2009M1d1)(y2008M2d1)]",
                       false, "Infinite Past", "Infinite Future");

  TestSharpDomain_Auto(2008, 12, 1, 16, 0, 0,
                       "[(y2008M12d31){d1}]",
                       false, "Infinite Past", "12/31/2008 00:00:00");
  TestSharpDomain_Auto(2008, 12, 31, 16, 0, 0,
                       "[(y2008M12d31){d1}]",
                       true, "12/31/2008 00:00:00", "1/1/2009 00:00:00");
  TestSharpDomain_Auto(2009, 1, 1, 0, 0, 0,
                       "[(y2008M12d31){d1}]",
                       false, "1/1/2009 00:00:00", "Infinite Future");


  Timezone tz = Timezone::PST();  // US Pacific Time
  LatLong p = LatLong::Create(37, -122);
  Place place(p, tz);

  TestGenericDomain_Auto(2008, 4, 9, 16, 0, 0, place, "[(d1){d1}]",
                         false, "4/2/2008 00:00:00", "5/1/2008 00:00:00");
  TestGenericDomain_Auto(2008, 4, 9, 16, 0, 0, place, "[(z1)(z2)]",
                         true, "4/9/2008 06:42:31", "4/9/2008 19:38:50");
  TestGenericDomain_Auto(2008, 4, 9, 16, 0, 0, place, "[(z1-m30){h1}]",
                         false, "4/9/2008 07:12:31", "4/10/2008 06:11:06");
  TestGenericDomain_Auto(2008, 4, 9, 16, 0, 0, place,
                         "[(t1z2-h1)(z2)] + [(t7z2-h1)(z2)]",
                         false, "4/6/2008 19:36:11", "4/12/2008 18:41:28");

  // test from command-line input (generic time-domain)
  LocalTime current(gFlag_year, gFlag_month, gFlag_day,
                    gFlag_hour, gFlag_minute, gFlag_second);
  LocalTime prev, next;
  string error_msg;

  // test GenericPoint from command line
  if (!gFlag_pointspec.empty()) {
    GenericPoint gp;
    ASSERT(gp.Init(gFlag_pointspec, &error_msg)) << error_msg;
    LOG(INFO) << "GenericPoint: " << gp.Print();
    LOG(INFO) << "Current: " << current.Print();
    gp.GetPrevious(current, &place, &prev);
    gp.GetNext(current, &place, &next);
    LOG(INFO) << "Previous point: " << prev.Print();
    LOG(INFO) << "Next point: " << next.Print();
  }

  // test TimeDomain<GenericPoint> from command line
  if (!gFlag_spec.empty()) {
    TimeDomain<GenericPoint> td;
    ASSERT(td.Init(gFlag_spec, &error_msg)) << error_msg;
    LOG(INFO) << "Time domain:\n\n" << td.Print() << "\n";
    bool in_domain = td.CheckTime(current, &place, &prev, &next);
    LOG(INFO) << "Current: " << current.Print()
           << " --- " << (in_domain ? "in domain" : "not in domain");
    LOG(INFO) << "Previous boundary: "
           << (prev.IsFinite() ? prev.Print() : "doesn't exist");
    LOG(INFO) << "Next boundary: "
           << (next.IsFinite() ? next.Print() : "doesn't exist");
  }
}

int init_main() {

  TestSharpPoint();

  TestTimeDomain();

  LOG(INFO) << "PASS";

  return 0;
}
