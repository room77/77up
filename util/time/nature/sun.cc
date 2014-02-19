#include "util/time/nature/sun.h"

namespace Nature {

// determine whether the current time is daytime
// if so, return true,
//        set prev_boundary to the last sunrise (<= current time)
//        set next_boundary to the next sunset (>= current time)
bool IsDayTime(Time t, const Place& place,
               Time *prev_boundary, Time *next_boundary,
               double *altitude) {
  Time prev_sunrise, prev_sunset, next_sunrise, next_sunset;
  bool is_daytime = CalculateSun(t, place,
                                 (prev_boundary ? &prev_sunrise : NULL),
                                 (prev_boundary ? &prev_sunset : NULL),
                                 (next_boundary ? &next_sunrise : NULL),
                                 (next_boundary ? &next_sunset : NULL),
                                 NULL, NULL,
                                 altitude, NULL, NULL);
  if (is_daytime) {
    // find previous sunrise and next sunset

    // previous sunrise
    if (prev_boundary)
      *prev_boundary = RefineSunriseTime(prev_sunrise, place, false);
    // next sunset
    if (next_boundary)
      *next_boundary = RefineSunsetTime(next_sunset, place, true);
  }
  else {
    // find previous sunset and next sunrise

    // previous sunset
    if (prev_boundary)
      *prev_boundary = RefineSunsetTime(prev_sunset, place, false);
    // next sunrise
    if (next_boundary)
      *next_boundary = RefineSunriseTime(next_sunrise, place, true);
  }

  return is_daytime;
}


// get more accurate sunrise time by refining twice
Time RefineSunriseTime(Time approx_sunrise, const Place& place,
                       bool towards_future) {
  Time refine1, refine2;

  CalculateSun(approx_sunrise, place,
               NULL, NULL, NULL, NULL,
               &refine1, NULL, NULL, NULL, NULL);
  CalculateSun(refine1, place,
               NULL, NULL, NULL, NULL,
               &refine2, NULL, NULL, NULL, NULL);
  if (refine2.IsFinite())
    return refine2;
  else
    return (towards_future ? Time::InfiniteFuture() : Time::InfinitePast());
}

// get more accurate sunset time by refining twice
Time RefineSunsetTime(Time approx_sunset, const Place& place,
                      bool towards_future) {
  Time refine1, refine2;

  CalculateSun(approx_sunset, place,
               NULL, NULL, NULL, NULL,
               NULL, &refine1, NULL, NULL, NULL);
  CalculateSun(refine1, place,
               NULL, NULL, NULL, NULL,
               NULL, &refine2, NULL, NULL, NULL);
  if (refine2.IsFinite())
    return refine2;
  else
    return (towards_future ? Time::InfiniteFuture() : Time::InfinitePast());
}

// given time t, calculate julian_day_minus_2451545, as well as
// the sun's right ascension (alpha) and declination (delta)
void CalculateSunParams(Time t,
                        double *julian_day_minus_2451545,
                        double *right_ascension, double *declination) {
  // constants
  static const double M0 = 357.5291 * PI / 180.0;
  static const double M1 = 0.98560028 * PI / 180.0;
  static const double C1 = 1.9148 * PI / 180.0;
  static const double C2 = 0.02 * PI / 180.0;
  static const double C3 = 0.0003 * PI / 180.0;
  static const double P = 102.9372 * PI / 180 + PI;
  static const double Epsilon =
    23.45 * PI / 180.0;  // obliquity of Earth's equator

  *julian_day_minus_2451545 = 1.0 * t.get_t() / 86400.0 - 10957.5;

  // mean anomaly
  double M = M0 + M1 * (*julian_day_minus_2451545);
  M = NormalizeAngle(M);

  // equation of center
  double C = C1 * sin(M) + C2 * sin(2 * M) + C3 * sin(3 * M);

  // true anomaly
  double nu = M + C;
  nu = NormalizeAngle(nu);

  // ecliptical longitude of the sun
  // (0 -> spring equinox, PI -> autumn equinox, northern hemisphere)
  double lambda_sun = nu + P;
  lambda_sun = NormalizeAngle(lambda_sun);

  // right ascension of the sun
  //   important: use atan2 instead of atan here; we want the result to
  //       be in the range of (-PI, PI), not (-PI/2, PI/2).
  *right_ascension = atan2(sin(lambda_sun) * cos(Epsilon), cos(lambda_sun));

  // declination of the sun
  *declination = asin(sin(lambda_sun) * sin(Epsilon));
}


// calculate certain data about the sun's position and movement
//   input:  Time and Place
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
                  Time *last_transit) {

  static const double horizon_adjustment = -0.83 * PI / 180.0;

  static const double Theta0 = 280.1600 * PI / 180.0;
  static const double Theta1 = 360.9856235 * PI / 180.0;

  // first, set default values
  if (approx_prev_sunrise != NULL)
    *approx_prev_sunrise = Time::InfinitePast();
  if (approx_next_sunrise != NULL)
    *approx_next_sunrise = Time::InfiniteFuture();
  if (nearby_sunrise != NULL)
    *nearby_sunrise = Time::InfiniteFuture();

  if (approx_prev_sunset != NULL)
    *approx_prev_sunset = Time::InfinitePast();
  if (approx_next_sunset != NULL)
    *approx_next_sunset = Time::InfiniteFuture();
  if (nearby_sunset != NULL)
    *nearby_sunset = Time::InfiniteFuture();

  if (altitude != NULL)
    *altitude = 0;
  if (azimuth != NULL)
    *azimuth = 0;
  if (last_transit != NULL)
    *last_transit = Time::InfinitePast();

  // check if input is a valid time
  if (!(t.IsFinite()))
    return false;

  double rad_latitude = place.get_latitude_radian();
  double rad_longitude = place.get_longitude_radian();

  //
  // calculation of approximate sunrise/sunset/transit times, based on
  //   http://www.astro.uu.nl/~strous/AA/en/reken/zonpositie.html
  //

  double julian_day_minus_2451545;
  double alpha_sun;  // right ascension of the sun
  double delta_sun;  // declination of the sun
  CalculateSunParams(t, &julian_day_minus_2451545, &alpha_sun, &delta_sun);

  // sidereal time
  double theta = Theta0 + Theta1 * julian_day_minus_2451545 + rad_longitude;
  theta = NormalizeAngle(theta);

  // hour angle:  how long ago (in sidereal time) the sun passed through
  //              the celestial meridian
  double H = NormalizeAngle(theta - alpha_sun);

  // calculate altitude of the sun  (in radians)
  double alt = asin(sin(rad_latitude) * sin(delta_sun) +
                    cos(rad_latitude) * cos(delta_sun) * cos(H));
  // VLOG(4) << "  altitude of the sun = " << alt * 180.0 / PI << " degrees.";
  bool is_daytime = (alt >= horizon_adjustment);

  if (altitude != NULL)
    *altitude = alt;

  if (azimuth != NULL) {
    // calculate azimuth of the sun  (in radians)
    //   important: use atan2 instead of atan here; we want the result to
    //       be in the range of (-PI, PI), not (-PI/2, PI/2).
    *azimuth = atan2(sin(H), (cos(H) * sin(rad_latitude) -
                              tan(delta_sun) * cos(rad_latitude)));
    // VLOG(4) << "  azimuth of the sun = " << (*azimuth) * 180.0 / PI
    //        << " degrees.";
  }

  if (last_transit != NULL)
    *last_transit = t - static_cast<int>(floor(H / Theta1 * 86400.0 + 0.5));

  // H_0:  H at sunset / sunrise time (time when h = -0.83 degrees)
  double cos_H_0 = (sin(horizon_adjustment) -
                    sin(rad_latitude) * sin(delta_sun))
                   / cos(rad_latitude) / cos(delta_sun);
  if (cos_H_0 >= -1.0 && cos_H_0 <= 1.0) {
    double H_0 = acos(cos_H_0);

    double sunrise_diff = NormalizeAngle(- H_0 - H);
    double sunset_diff  = NormalizeAngle(H_0 - H);

    // calculate nearby sunrise time (refined)
    double diff1 = sunrise_diff;
    if (diff1 > PI)
      diff1 -= 2 * PI;
    Time nearby_sunrise_time =
      t + static_cast<int> (floor(diff1 / Theta1 * 86400.0 + 0.5));
    if (nearby_sunrise != NULL)
      *nearby_sunrise = nearby_sunrise_time;

    // calculate nearby sunset time (refined)
    double diff2 = sunset_diff;
    if (diff2 > PI)
      diff2 -= 2 * PI;
    Time nearby_sunset_time =
        t + static_cast<int> (floor(diff2 / Theta1 * 86400.0 + 0.5));
    if (nearby_sunset != NULL)
      *nearby_sunset = nearby_sunset_time;

    if (approx_prev_sunrise != NULL) {
      // calculate previous sunrise time (approximate)
      // (<= current time)
      if (nearby_sunrise_time == t)
        *approx_prev_sunrise = t;  // current time is sunrise time
      else {
        double diff = sunrise_diff;
        if (diff > 0)
          diff -= 2 * PI;
        *approx_prev_sunrise =
          t + static_cast<int> (floor(diff / Theta1 * 86400.0 + 0.5));
      }
    }

    if (approx_next_sunrise != NULL) {
      // calculate next sunrise time (approximate)
      // (>= current time)
      if (nearby_sunrise_time == t)
        *approx_next_sunrise = t;  // current time is sunrise time
      else {
        double diff = sunrise_diff;
        *approx_next_sunrise =
          t + static_cast<int> (floor(diff / Theta1 * 86400.0 + 0.5));
      }
    }

    if (approx_prev_sunset != NULL) {
      // calculate previous sunset time (approximate)
      // (<= current time)
      if (nearby_sunset_time == t)
        *approx_prev_sunset = t;  // current time is sunset time
      else {
        double diff = sunset_diff;
        if (diff > 0)
          diff -= 2 * PI;
        *approx_prev_sunset =
          t + static_cast<int> (floor(diff / Theta1 * 86400.0 + 0.5));
      }
    }

    if (approx_next_sunset != NULL) {
      // calculate next sunset time (approximate)
      // (>= current time)
      if (nearby_sunset_time == t)
        *approx_next_sunset = t;  // current time is sunset time
      else {
        double diff = sunset_diff;
        *approx_next_sunset =
          t + static_cast<int> (floor(diff / Theta1 * 86400.0 + 0.5));
      }
    }
  }
  else {
    // no sunset / sunrise

    // do nothing -- use default values
  }

  return is_daytime;
}



}

