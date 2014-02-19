#ifndef _PUBLIC_UTIL_TIME_TIMEZONE_H_
#define _PUBLIC_UTIL_TIME_TIMEZONE_H_

#include "base/common.h"
#include "util/serial/serializer.h"
#include "util/memory/collection.h"
#include "util/index/index.h"
#include "util/time/localtime.h"
#include "util/time/sharppoint.h"
#include "util/time/timedomain.h"
#include <time.h>

// --------------------------------------------------------------------

//
// TimezoneSpec gives details of a timezone, including a unique ID,
// descriptive name, offset from GMT, DST offset and DST time domain
// (string and object).
// It has a serializable signature of three fields that uniquely defines
// a timezone (gmt_offset_, dst_offset_ and dst_domain_string_).
//

//
// In KnownTimezones class, we keep track of all known timezones in the world.
// Each known timezone is assigned a unique integer ID which may be used
// by other datatypes throughout our code base.
//
// IMPORTANT: Existing timezones and their IDs (see Init()) may not be changed.
//            New timezones may be added to Init() with new IDs.
//

//
// Timezone class contains the short timezone ID only, and provides methods
// to do timezone-related calculations.
//

namespace TZ {
  //
  // DST domain definitions for various countries/regions
  //
  // empty domain
  extern const char gDSTDomain_None[];
  // USA
  extern const char gDSTDomain_US[];
  // most of Canada
  extern const char gDSTDomain_Canada[];
  // DST start/end times in Newfoundland and Labrador, Canada is
  // slightly different from the rest of US/Canada
  extern const char gDSTDomain_Newfoundland[];

  // given a country name and a state name, return its DST domain string
  string GetDSTDomain(const string& country_name, const string& state_name);
}


class TimezoneSpec {
 public:
  // Default constructor.
  TimezoneSpec() {}

  //
  // there are two ways of initializing a timezone:
  // 1. by GMT offset (# of seconds), DST offset (# of seconds) and DST domain
  // 2. by GMT offset (string), DST offset (# of minutes) and DST domain
  // The second format is for the convenience of GDF data parsers.
  //
  TimezoneSpec(short id, const string& name,
               const string& abbr_standard, const string& abbr_dst,
               int gmt_offset, int dst_offset, const string& dst_domain_str)
    : id_(id), name_(name),
      abbr_standard_(abbr_standard), abbr_dst_(abbr_dst) {
    SetTimezone(gmt_offset, dst_offset, dst_domain_str);
  }
  TimezoneSpec(short id, const string& name,
               const string& abbr_standard, const string& abbr_dst,
               const string& zone_offset, int dst_offset_minutes,
               const string& dst_domain_str)
    : id_(id), name_(name),
      abbr_standard_(abbr_standard), abbr_dst_(abbr_dst) {
    SetTimezone(zone_offset, dst_offset_minutes, dst_domain_str);
  }

  void SetTimezone(int gmt_offset, int dst_offset,
                   const string& dst_domain_str);
  void SetTimezone(const string& zone_offset, int dst_offset_minutes,
                   const string& dst_domain_str);

  ~TimezoneSpec() {};

  inline short get_id() const    { return id_; }
  inline string get_name() const { return name_; }
  inline string get_abbr(bool is_dst) const {
    return (is_dst ? abbr_dst_ : abbr_standard_);
  }

  // conversion from local time to universal time
  // (if parameter in_dst is non-NULL, it will be set to indicate whether
  //  the given localtime was interpreted as DST or not)
  time_t FromLocalTime(const LocalTime& localtime,
                       bool *in_dst = NULL) const;
  // conversion from universal time to local time
  // (optional "offset" argument returns actual offset from GMT, in seconds)
  void ToLocalTime(time_t utime, LocalTime *localtime,
                   int *offset = NULL, time_t *next_dst_boundary = NULL) const;

  // check if the given local time has DST in effect
  bool InDST(const LocalTime& localtime) const;

 private:
  short id_;
  string name_, abbr_standard_, abbr_dst_;
  int gmt_offset_;  // offset from GMT, in seconds
  int dst_offset_;   // daylight-saving-time offset, in seconds
  string dst_domain_string_;  // DST time domain string
  TimeDomain<SharpPoint> dst_domain_;  // DST time domain object
 public:
  SIGNATURE(gmt_offset_*1 / dst_offset_*2 / dst_domain_string_*3);
};


class KnownTimezones {
 public:
  ~KnownTimezones() {};
  static const KnownTimezones& Instance();

  inline const TimezoneSpec *Lookup(const TimezoneSpec& tz) const {
    return timezone_collection_.Lookup(tz);
  }
  inline const TimezoneSpec *LookupByID(short tz_id) const {
    return tz_index_by_id_.RetrieveUnique(tz_id);
  }

 protected:
  KnownTimezones() { Init(); }

 private:
  static KnownTimezones *the_one_;

  // add all known timezones
  void Init();

  // add a timezone to our collection
  void AddTimezone(short id, const string& name,
                   const string& abbr_standard, const string& abbr_dst,
                   const string& zone_offset, int dst_offset_minutes,
                   const string& dst_domain_str);

 public:
  UniqueCollection<TimezoneSpec> timezone_collection_;
  Index<short, const TimezoneSpec *> tz_index_by_id_;
};


class Timezone {
 public:
  Timezone() : tz_id_(-1) {
    static_assert(sizeof(Timezone) == 2, "Invalid Timezone size");
  }
  Timezone(const string& zone_offset, int dst_offset_minutes,
           const string& time_domain) {
    Init(zone_offset, dst_offset_minutes, time_domain);
  }
  Timezone(int gmt_offset, int dst_offset_minutes,
           const string& time_domain) {
    Init(gmt_offset, dst_offset_minutes, time_domain);
  }
  ~Timezone() {};

  void Clear() { tz_id_ = -1; }

  inline void Init(const string& zone_offset, int dst_offset_minutes,
                   const string& time_domain) {
    TimezoneSpec z(-1, "", "", "",
                   zone_offset, dst_offset_minutes, time_domain);
    InitTZ(z);
  }
  inline void Init(int gmt_offset, int dst_offset, const string& time_domain) {
    TimezoneSpec z(-1, "", "", "",
                   gmt_offset, dst_offset, time_domain);
    InitTZ(z);
  }
  inline void InitTZ(const TimezoneSpec& tz) {
    // look up the timezone in our list of known timezones
    const TimezoneSpec *known_tz = KnownTimezones::Instance().Lookup(tz);
    ASSERT(known_tz != NULL)
      << "Unknown timezone: " << tz.ToJSON()
      << "\nPlease add this timezone to KnownTimezones::Init()";
    // extract the timezone ID
    tz_id_ = known_tz->get_id();
  }

  inline short get_tz_id() const { return tz_id_; }

  inline const TimezoneSpec *GetTimezoneSpec() const {
    const TimezoneSpec *tz = KnownTimezones::Instance().LookupByID(tz_id_);
    ASSERT(tz != NULL) << "Cannot find timezone with ID " << tz_id_;
    return tz;
  }

  inline string GetTimezoneName() const {
    return GetTimezoneSpec()->get_name();
  }
  inline string GetTimezoneAbbr(bool is_dst) const {
    return GetTimezoneSpec()->get_abbr(is_dst);
  }

  inline static int GetApproxGMTOffset(double longitude) {
    int approx_offset = static_cast<int>((longitude + 7.5) / 15);
    return approx_offset;
  }

  // conversion from local time to universal time
  // (if parameter in_dst is non-NULL, it will be set to indicate whether
  //  the given localtime was interpreted as DST or not)
  inline time_t FromLocalTime(const LocalTime& localtime,
                              bool *in_dst = NULL) const {
    return GetTimezoneSpec()->FromLocalTime(localtime, in_dst);
  }
  // conversion from universal time to local time
  inline void ToLocalTime(time_t utime, LocalTime *localtime,
                          int *offset = NULL,
                          time_t *next_dst_boundary = NULL) const {
    GetTimezoneSpec()->ToLocalTime(utime, localtime, offset, next_dst_boundary);
  }
  inline LocalTime ToLocalTime(time_t utime) const {
    LocalTime local;
    ToLocalTime(utime, &local, NULL);
    return local;
  }

  // check if the given local time has DST in effect
  inline bool InDST(const LocalTime& localtime) const {
    return GetTimezoneSpec()->InDST(localtime);
  }

  // return a static Pacific Time object
  static Timezone PST() {
    static Timezone tz("GMT-8:00", 60, TZ::gDSTDomain_US);
    return tz;
  }
  // return a static Eastern Time object
  static Timezone EST() {
    static Timezone tz("GMT-5:00", 60, TZ::gDSTDomain_US);
    return tz;
  }
  // return a static Greenwich Mean Time object
  static Timezone GMT() {
    static Timezone tz("GMT+0:00", 0, TZ::gDSTDomain_None);
    return tz;
  }

  // convert 00:00:00 and 23:59:59 local time to universal time
  inline time_t DayBeginning(const LocalDate& d) const {
    return FromLocalTime(LocalTime(d));
  }
  inline time_t DayEnd(const LocalDate& d) const {
    return FromLocalTime(LocalTime(d, 23, 59, 59));
  }

  inline bool Valid() const {
    return (tz_id_ >= 0);
  }

  inline bool operator==(const Timezone& t) const {
    return (tz_id_ == t.tz_id_);
  }
  inline bool operator!=(const Timezone& t) const {
    return (tz_id_ != t.tz_id_);
  }
 private:
  short tz_id_;

 public:
  SERIALIZE(tz_id_*1);
};


#endif  // _PUBLIC_UTIL_TIME_TIMEZONE_H_
