#include "util/time/timezone.h"


namespace TZ {

  // empty domain
  const char gDSTDomain_None[] = "";
  // USA
  const char gDSTDomain_US[] = "[(M3f21h2)(M11f11h2)]";
  // most of Canada
  const char gDSTDomain_Canada[] = "[(M3f21h2)(M11f11h2)]";
  // DST start/end times in Newfoundland and Labrador, Canada is
  // slightly different from the rest of US/Canada
  const char gDSTDomain_Newfoundland[] = "[(M3f21h0m1)(M11f11h0m1)]";

  // given a country name and a state name, return its DST domain string
  string GetDSTDomain(const string& country_name, const string& state_name) {
    if (country_name == "United States")
      return gDSTDomain_US;
    else if (country_name == "Canada") {
      if (state_name.find("Newfoundland") == 0) {
        // Newfoundland and Labrador province
        return gDSTDomain_Newfoundland;
      }
      else
        return gDSTDomain_Canada;
    }
    else {
      ASSERT(false) << "Timezone not defined for country " << country_name
                    << ", state " << state_name;
      return "";  // dummy statement to make compiler happy
    }
  }
}


//
// initialize KnowTimezones collection
//

KnownTimezones *KnownTimezones::the_one_ = NULL;

const KnownTimezones& KnownTimezones::Instance() {
  static KnownTimezones the_one;
  return the_one;
}

// Add all known timezones
//
// (Note: duplicate timezone specs are allowed, in which case multiple IDs
//        are indexed to the same timezone.  Each timezone only has one
//        "primary" ID, which is the first ID assigned to it.)
void KnownTimezones::Init() {
  // add U.S. timezones
  AddTimezone(1, "Pacific", "PST", "PDT", "GMT-08:00", 60, TZ::gDSTDomain_US);
  AddTimezone(2, "Mountain", "MST", "MDT", "GMT-07:00", 60, TZ::gDSTDomain_US);
  AddTimezone(3, "Arizona", "MST", "MST", "GMT-07:00", 0,  TZ::gDSTDomain_US);
  AddTimezone(4, "Central", "CST", "CDT", "GMT-06:00", 60, TZ::gDSTDomain_US);
  AddTimezone(5, "Eastern", "EST", "EDT", "GMT-05:00", 60, TZ::gDSTDomain_US);
  AddTimezone(6, "Alaska", "AKST", "AKDT", "GMT-09:00", 60, TZ::gDSTDomain_US);
  AddTimezone(7, "Hawaii", "HST", "HST", "GMT-10:00", 0,  TZ::gDSTDomain_US);

  AddTimezone(8, "Pacific", "PST", "PST",
              "GMT-08:00", 0, TZ::gDSTDomain_Canada);
  AddTimezone(9, "Saskatchewan", "CST", "CST",
              "GMT-06:00", 0,  TZ::gDSTDomain_Canada);
  AddTimezone(10, "Atlantic", "AST", "ADT",
              "GMT-04:00", 60, TZ::gDSTDomain_Canada);

  // Newfoundland Time (Canada)
  AddTimezone(11, "Newfoundland", "NST", "NDT",
              "GMT-03:30", 60, TZ::gDSTDomain_Newfoundland);
  // most of Labrador is on Atlantic time
  AddTimezone(12, "Atlantic/Labrador", "AST", "ADT",
              "GMT-04:00", 60, TZ::gDSTDomain_Newfoundland);

  AddTimezone(13, "Eastern w/o DST", "EST", "EST",
              "GMT-05:00", 0, TZ::gDSTDomain_Canada);

  AddTimezone(14, "Puerto Rico", "AST", "AST",
              "GMT-04:00", 0, TZ::gDSTDomain_US);
  AddTimezone(15, "Micronesia", "GMT+11", "GMT+11",
              "GMT+11:00", 0, TZ::gDSTDomain_US);
  AddTimezone(16, "American Samoa", "GMT-11", "GMT-11",
              "GMT-11:00", 0, TZ::gDSTDomain_US);
  AddTimezone(17, "Palau", "PWT", "PWT",
              "GMT+09:00", 0, TZ::gDSTDomain_US);
  AddTimezone(18, "Guam", "GST", "GST",
              "GMT+10:00", 0, TZ::gDSTDomain_US);
  AddTimezone(19, "Marshall Islands", "MHT", "MHT",
              "GMT+12:00", 0, TZ::gDSTDomain_US);

  // our postal code db seems to have incorrect timezone info for
  // Micronesia and Palau
  AddTimezone(21, "American Samoa", "GMT-11", "GMT-11",
              "GMT-11:00", 60, TZ::gDSTDomain_US);  // wrong? no DST
  AddTimezone(22, "Palau", "PWT", "PWT",
              "GMT+09:00", 60, TZ::gDSTDomain_US);  // wrong? no DST

  // Greenwich mean time
  AddTimezone(20, "GMT", "GMT", "GMT",
              "GMT+00:00", 0, TZ::gDSTDomain_None);

  VLOG(2) << timezone_collection_.size() << " timezones registered.";
}

// add a timezone to our collection
void KnownTimezones::AddTimezone(short id, const string& name,
                                 const string& abbr_standard,
                                 const string& abbr_dst,
                                 const string& zone_offset,
                                 int dst_offset_minutes,
                                 const string& dst_domain_str) {
  ASSERT(id >= 0) << "timezone ID cannot be negative";
  TimezoneSpec tz(id, name, abbr_standard, abbr_dst,
                  zone_offset, dst_offset_minutes, dst_domain_str);

  // store the timezone and create an index on its ID

  const TimezoneSpec *stored = timezone_collection_.Lookup(tz);
  if (stored == NULL)
    stored = timezone_collection_.Store(tz);
  else {
    VLOG(2) << "Timezone " << id << " is the same as timezone "
           << stored->get_id() << ": " << stored->ToJSON();
  }

  ASSERT(tz_index_by_id_.RetrieveUnique(id) == NULL)
    << "Duplicate timezone ID " << id;

  tz_index_by_id_.AddToIndex(id, stored);
}



void TimezoneSpec::SetTimezone(int gmt_offset, int dst_offset,
                               const string& dst_domain_str) {
  gmt_offset_ = gmt_offset;
  dst_offset_ = dst_offset;
  dst_domain_string_ = dst_domain_str;
  // parse DST domain string into TimeDomain object
  string error_msg;
  ASSERT(dst_domain_.Init(dst_domain_str, &error_msg))
    << "Error parsing time domain " << dst_domain_str << ": " << error_msg;
}

void TimezoneSpec::SetTimezone(const string& zone_offset,
                               int dst_offset_minutes,
                               const string& dst_domain_str) {
  // example of a specification for US Pacific Time:
  //   zone_offset: GMT-8:00
  //   dst_offset:  60
  //   time_domain: [(M3f21h2)(M11f11h2)]
  ASSERT(zone_offset.size() >= 5 &&
         zone_offset[0] == 'G' &&
         zone_offset[1] == 'M' &&
         zone_offset[2] == 'T')
           << "time zone offset string must begin with GMT: " << zone_offset;

  ASSERT(zone_offset[3] == '+' || zone_offset[3] == '-')
    << "time zone offset must begin with GMT+ or GMT-";

  int offset_sign = (zone_offset[3] == '+' ? 1 : -1);

  // parse to get hour and minute offset
  const char *hour_start = zone_offset.c_str() + 4; // skip leading "GMT+/-"
  char *endptr;
  int hour_offset = strtol(hour_start, &endptr, 10);
  ASSERT(endptr > hour_start) << "Error paring timezone: " << zone_offset;

  int minute_offset = 0;
  if (*endptr == ':') {
    const char *min_start = endptr + 1;
    minute_offset = strtol(min_start, &endptr, 10);
    ASSERT(endptr > min_start) << "Error parsing timezone: " << zone_offset;
  }
  ASSERT(hour_offset >=0 && hour_offset < 24 &&
         minute_offset >= 0 && minute_offset < 60);

  SetTimezone((hour_offset * ONE_HOUR + minute_offset * ONE_MINUTE)
              * offset_sign,
              dst_offset_minutes * ONE_MINUTE,
              dst_domain_str);
}

// conversion routines between local time and universal time

// dst_domain_ specifies the validity of daylight saving time in terms of
// local time.  Start time is in standard time, while end time is in DST,
// For example, in the U.S., DST validity can be specified as
//   [(M3f21h2)(M11f11h2)]
//   -- from (2am, second Sunday of March) to (2am, first Sunday of November)
// The first "2am" refers to standard time, but the second "2am" refers to DST.

// conversion from LocalTime to universal time_t
//
// Special note for times around DST boundaries:
//   1. On the day DST begins, there is a range of invalid LocalTime, but
//      we make no attempt to detect such errors, and simply proceed
//      with normal calculation.  (As a result, if caller specifies 2:30am
//      on the day DST begins in the U.S., we may treated it as if it were
//      1:30am standard time.)
//   2. On the day DST ends, there is a range of ambiguous LocalTime.  We
//      choose standard time, unless the input LocalTime has is_dst_ field
//      set, in which case we follow is_dst_ field.  (Except in this
//      particular corner case, we always ignore is_dst_ field and determine
//      DST algorithmically.)
//
// U.S. DST example:
//
//    DST begins (spring)                              DST ends (fall)
//        2am 3am              ***DST***                1am 2am
// --------....-----------------------------------------------
//                                                        -------------------
//          ^ gap                                          ^ ambiguous
//
time_t TimezoneSpec::FromLocalTime(const LocalTime& localtime,
                                   bool *in_dst) const {
  if (localtime.IsInfinitePast())
    return INFINITE_PAST;
  else if (localtime.IsInfiniteFuture())
    return INFINITE_FUTURE;
  else {
    bool dst = InDST(localtime);
    if (in_dst != NULL)
      *in_dst = dst;
    LocalTime gmt = (dst ?
                     localtime - gmt_offset_ - dst_offset_ :
                     localtime - gmt_offset_);
    return gmt.SecondsSince1970();
  }
}

// conversion from universal time_t to LocalTime
// (optional output fields:
//    offset: offset from GMT
//    next_dst_boundary: universal time of the closest DST boundary that is
//                       >= the given time (utime).
//                       if DST does not apply, this parameter is set to 0.
void TimezoneSpec::ToLocalTime(time_t utime, LocalTime *localtime,
                               int *offset, time_t *next_dst_boundary) const {
  if (offset)
    *offset = gmt_offset_;
  if (next_dst_boundary)
    *next_dst_boundary = 0;

  if (utime <= INFINITE_PAST)
    *localtime = LocalTime::InfinitePast();
  else if (utime >= INFINITE_FUTURE)
    *localtime = LocalTime::InfiniteFuture();
  else {
    localtime->ElapseFrom1970(utime + gmt_offset_);
    // localtime is now in standard time (non-DST) of this time zone

    bool in_dst = false;
    if (dst_offset_ != 0) {
      LocalTime next_boundary;
      in_dst = dst_domain_.CheckTime(*localtime, NULL, NULL, &next_boundary);

      // compensate for misclassification just after DST ends (because
      // dst_domain_ ending time is specified in DST, not standard time)
      if (in_dst && (*localtime) + dst_offset_ >= next_boundary) {
        in_dst = false;
        // the next DST boundary was also calculated incorrectly and needs
        // to be compensated here, in case it's needed
        LocalDate dummy = *localtime;
        dst_domain_.CheckTime(dummy + 1, NULL, NULL, &next_boundary);
      }

      if (in_dst) {
        (*localtime) += dst_offset_;
        if (offset)
          (*offset) += dst_offset_;
      }

      if (next_dst_boundary) {
        // optional request: figure out when the next DST boundary is
        next_boundary.IncrementSecond(-1);  // 1 second before DST change
        next_boundary.set_is_dst(in_dst ? 1 : 0);
        *next_dst_boundary = FromLocalTime(next_boundary) + 1;
      }
    }
    localtime->set_is_dst(in_dst ? 1 : 0);
  }
}

// check if the given local time has DST in effect
bool TimezoneSpec::InDST(const LocalTime& localtime) const {
  // To see if the given localtime is in DST, we check it against dst_domain_
  bool in_dst = false;
  if (dst_offset_ != 0) {
    LocalTime next_boundary;
    in_dst = dst_domain_.CheckTime(localtime, NULL, NULL, &next_boundary);
    if (in_dst &&
        (localtime + dst_offset_ >= next_boundary)) {
      // we happen to be in the ambiguous range around the time DST ends
      // (see note 2 above)
      // --- follow is_dst_ field if it's set; otherwise, assume standard time
      in_dst = (localtime.is_dst() == 1);
    }
  }
  return in_dst;
}

