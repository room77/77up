#ifndef _PUBLIC_UTIL_TIME_NATURE_SUN_H_
#define _PUBLIC_UTIL_TIME_NATURE_SUN_H_

#include "util/time/utime.h"
#include "util/geo/place.h"

namespace Nature {

  // determine whether the current time is daytime
  // if so, return true,
  //        set prev_boundary to the last sunrise (<= current time)
  //        set next_boundary to the next sunset (>= current time)
  bool IsDayTime(Time t, const Place& place,
                 Time *prev_boundary, Time *next_boundary,
                 double *altitude);

  // get more accurate sunrise time by refining twice
  Time RefineSunriseTime(Time approx_sunrise, const Place& place,
                         bool towards_future);

  // get more accurate sunset time by refining twice
  Time RefineSunsetTime(Time approx_sunset, const Place& place,
                        bool towards_future);

  // given time t, calculate julian_day_minus_2451545, as well as
  // the sun's right ascension (alpha) and declination (delta)
  void CalculateSunParams(Time t,
                          double *julian_day_minus_2451545,
                          double *right_ascension, double *declination);

  // calculate certain data about the sun's position and movement
  //   input:  TimePlace object
  //   output: (pass in NULL for each field to ignore)
  //           previous sunrise time (approximate)
  //           previous sunset time (approximate)
  //           next sunrise time (approximate)
  //           next sunset time (approximate)
  //           nearby sunrise time (refined)
  //           nearby sunset time (refined)
  //           altitude of the sun
  //           azimuth of the sun
  //           last transit time
  bool CalculateSun(Time t, const Place& place,
                    Time *approx_prev_sunrise, Time *approx_prev_sunset,
                    Time *approx_next_sunrise, Time *approx_next_sunset,
                    Time *nearby_sunrise, Time *nearby_sunset,
                    double *altitude, double *azimuth,
                    Time *last_transit);

  // normalize angle to between [0, 2*PI)
  inline double NormalizeAngle(double angle) {
    return (angle - floor(angle / PI / 2.0) * PI * 2.0);
  }

  // get previous sunrise time
  inline Time PreviousSunrise(Time t, const Place& place) {
    Time prev_sunrise;
    CalculateSun(t, place,
                 &prev_sunrise, NULL, NULL, NULL,
                 NULL, NULL, NULL, NULL, NULL);
    return RefineSunriseTime(prev_sunrise, place, false);
  }

  // get previous sunset time
  inline Time PreviousSunset(Time t, const Place& place) {
    Time prev_sunset;
    CalculateSun(t, place,
                 NULL, &prev_sunset, NULL, NULL,
                 NULL, NULL, NULL, NULL, NULL);
    return RefineSunsetTime(prev_sunset, place, false);
  }

  // get next sunrise time
  inline Time NextSunrise(Time t, const Place& place) {
    Time next_sunrise;
    CalculateSun(t, place,
                 NULL, NULL, &next_sunrise, NULL,
                 NULL, NULL, NULL, NULL, NULL);
    return RefineSunriseTime(next_sunrise, place, true);
  }

  // get next sunset time
  inline Time NextSunset(Time t, const Place& place) {
    Time next_sunset;
    CalculateSun(t, place,
                 NULL, NULL, NULL, &next_sunset,
                 NULL, NULL, NULL, NULL, NULL);
    return RefineSunsetTime(next_sunset, place, true);
  }

}

#endif  // _PUBLIC_UTIL_TIME_NATURE_SUN_H_
