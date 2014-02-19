#ifndef _PUBLIC_UTIL_GEO_PLACE_H_
#define _PUBLIC_UTIL_GEO_PLACE_H_

#include "base/common.h"
#include "util/time/timezone.h"
#include "util/time/utime.h"
#include "util/geo/latlong.h"

class Place {
 public:
  Place(const LatLong *latlong, Timezone tz) :
    latlong_(*latlong), tz_(tz) {};
  Place(const LatLong& latlong, Timezone tz) :
    latlong_(latlong), tz_(tz) {};
  ~Place() {};

  inline const LatLong *get_latlong() const { return &latlong_; }
  inline Timezone get_tz() const { return tz_; }

  // get latitude/longitude in radians
  // (-pi to pi for longitude, -pi/2 to pi/2 for latitude)
  inline double get_latitude_radian() const  {
    return latlong_.get_radian_latitude();
  }
  inline double get_longitude_radian() const {
    return latlong_.get_radian_longitude();
  }

  // human-readable string of local time
  inline string PrintTime(Time t) const { return t.Print(tz_); }

  // get local time
  LocalTime GetLocalTime(Time t) const { return t.GetLocalTime(tz_); }

  // get universal time
  Time GetUniversalTime(const LocalTime& lt) const {
    return Time(lt, tz_);
  }

 private:
  LatLong latlong_;
  Timezone tz_;
};

#endif  // _PUBLIC_UTIL_GEO_PLACE_H_
