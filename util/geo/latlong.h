#ifndef _PUBLIC_UTIL_GEO_LATLONG_H_
#define _PUBLIC_UTIL_GEO_LATLONG_H_

#include "base/common.h"
#include "base/extra_defs.h"
#include "util/serial/serializer.h"

// LatLong class represents a latitude/longitude pair
// stored as actual value times 10^7
// On dev, it will assert when you try to initialize with an invalid lat/lon.
// On production, you should check the Valid() function to verify the lat/lon is valid

constexpr int kLatLongMultiplier = 10000000;

// factor to convert from Mercator result to pixel value
// (at zoom level 0, 1 pixel equals to this much in Mercator scale (radian))
constexpr float kGMapPixelRatio_X = 0.02;
constexpr float kGMapPixelRatio_Y = 0.02;

class LatLong {
 public:
  static bool Validate(double *lat, double *lon) {
    if (*lat >= -90 && *lat <= 90 && *lon >= -180 && *lon <= 180) return true;
    *lat = *lon = 0;
    return false;
  }

  static LatLong Undefined() {
    LatLong undefined;
    return undefined;
  }

  // after initialization, use Valid() to check if it is a valid lat/lon
  static LatLong Create(double latitude_d, double longitude_d) {
    LatLong p;
    p.set_degree_latlong(latitude_d, longitude_d);
    return p;
  }

  inline bool Valid() const {
    return (abs(latitude_) <= 90 * kLatLongMultiplier &&
            abs(longitude_) <= 180 * kLatLongMultiplier);
  }

  // DOES NOT ASSERT. need this to check if a lat long is
  // valid before creating
  static bool IsValid(double latitude_d, double longitude_d) {
    if (fabs(latitude_d) <= 90) {
      if (fabs(longitude_d) > 180) {
        longitude_d = NormalizeLongitude(longitude_d);
      }
      if (fabs(longitude_d) <= 180) {
        return true;
      }
    }
    return false;
  }

  inline static double NormalizeLongitude(double longitude) {
    return (longitude - (floor((longitude + 180) / 180 / 2) * 180 * 2));
  }

  // from an exact bounding box (specified by northwest/southeast corners),
  // calculate extended bounding box
  // (note: we ignore cases where the bounding box crosses 180-degree longitude)
  static void GetExtendedBoundingBox(const LatLong& nw, const LatLong& se,
                                     LatLong *e_nw, LatLong *e_se);

  // start from the given (northwest, southeast) pair, expand it if necessary
  // to accomodate another bounding box (new_nw, new_se)
  // (note: we ignore cases where the bounding box crosses 180-degree longitude)
  static void ExpandBoundingBox(const pair<double, double>& new_nw,
                                const pair<double, double>& new_se,
                                pair<double, double> *northwest,
                                pair<double, double> *southeast);

  inline bool InBoundingBox(const LatLong& northwest,
                            const LatLong& southeast) const {
    // ignore bounding boxes that cross 180-degree longitude line
    // todo: handle this special corner case
    if (northwest.get_multiplied_longitude() > southeast.get_multiplied_longitude())
      return true;
    return (latitude_ >= southeast.get_multiplied_latitude() &&
            latitude_ <= northwest.get_multiplied_latitude() &&
            longitude_ >= northwest.get_multiplied_longitude() &&
            longitude_ <= southeast.get_multiplied_longitude());
  }

  // Note: This expects the lat long values have already been multiplied.
  // *DO NOT* use this function if you have lat longs in degrees. Use set_latlong_d in that case.
  inline void set_multiplied_latlong(int latitude, int longitude) {
    latitude_ = latitude;
    longitude_ = longitude;
    ASSERT_DEV(abs(latitude_) <= 90 * kLatLongMultiplier)
      << "latitude out of range: " << latitude_;
    ASSERT_DEV(abs(longitude_) <= 180 * kLatLongMultiplier)
      << "longitude out of range: " << longitude_;
  }

  inline int get_multiplied_latitude() const  { return latitude_; }
  inline int get_multiplied_longitude() const { return longitude_; }

  // set latitude/longitude values (input parameters are real lat/long values in degrees,
  // as floating-point ("double") numbers)
  inline void set_degree_latlong(double latitude_d, double longitude_d) {
    ASSERT_DEV(fabs(latitude_d) <= 90)
      << "latitude out of range: " << latitude_d;

    if (fabs(longitude_d) > 180) {
      // normalize longitude to [-180,180]
      longitude_d = NormalizeLongitude(longitude_d);
      ASSERT_DEV(fabs(longitude_d) <= 180);
    }

    latitude_ = static_cast<int>(latitude_d * kLatLongMultiplier);
    longitude_ = static_cast<int>(longitude_d * kLatLongMultiplier);
  }

  // get floating point latitude/longitude values
  inline double get_degree_latitude() const  {
    ASSERT(abs(latitude_) <= 90 * kLatLongMultiplier)
      << "latitude out of range: " << latitude_;
    return (static_cast<double>(latitude_) / static_cast<double>(kLatLongMultiplier));
  }

  inline double get_degree_longitude() const {
    ASSERT(abs(longitude_) <= 180 * kLatLongMultiplier)
      << "longitude out of range: " << longitude_;
    return (static_cast<double>(longitude_) / static_cast<double>(kLatLongMultiplier));
  }

  // initialize LatLong object from lat/long specification in radians
  inline void set_radian_latlong(double lat_radian, double long_radian) {
    ASSERT(fabs(lat_radian) <= PI / 2);  // lat radian must be in [-PI/2, PI/2]
    if (fabs(long_radian) > PI) {
      // normalize longitude radian to [-PI,PI]
      long_radian -= (floor((long_radian + PI) / PI / 2) * PI * 2);
      ASSERT(fabs(long_radian) <= PI);
    }
    set_degree_latlong(lat_radian * 180 / PI, long_radian * 180 / PI);
  }

  // get floating point latitude/longitude values, as radians
  // (-pi to pi for longitude, -pi/2 to pi/2 for latitude)
  inline double get_radian_latitude() const  {
    ASSERT(abs(latitude_) <= 90 * kLatLongMultiplier)
      << "latitude out of range: " << latitude_;
    return (static_cast<double>(PI) / 180.0
            / static_cast<double>(kLatLongMultiplier) * latitude_);
  }
  inline double get_radian_longitude() const {
    ASSERT(abs(longitude_) <= 180 * kLatLongMultiplier)
      << "longitude out of range: " << longitude_;
    return (static_cast<double>(PI) / 180.0
            / static_cast<double>(kLatLongMultiplier) * longitude_);
  }

  // Calculate the surface distance of two points, assuming the earth is
  // a sphere with radius EARTH_RADIUS
  // (result is in kilometers)
  static double SurfaceDistance(const LatLong& p1, const LatLong& p2);
  inline static double SurfaceDistance(const LatLong *p1, const LatLong *p2) {
    return SurfaceDistance(*p1, *p2);
  }

  inline double SurfaceDistanceFrom(const LatLong& other) const {
    return SurfaceDistance(*this, other);
  }

  inline double SurfaceDistanceFrom(const LatLong *other) const {
    return SurfaceDistance(*this, *other);
  }

  inline bool operator==(const LatLong& other) const {
    return (latitude_ == other.get_multiplied_latitude() &&
            longitude_ == other.get_multiplied_longitude());
  }

  // calculate the direction from src to dest
  // result unit is in radian:
  //   0      -- north
  //   pi/2   -- east
  //   pi     -- south
  //   pi*1.5 -- west
  static double RadianDirection(const LatLong& src, const LatLong& dest);
  inline static double RadianDirection(const LatLong *src, const LatLong *dest) {
    return RadianDirection(*src, *dest);
  }

  inline double RadianDirectionTo(const LatLong& other) const {
    return RadianDirection(*this, other);
  }

  inline double RadianDirectionTo(const LatLong *other) const {
    return RadianDirection(*this, *other);
  }

  inline pair<double, double> ToMercator(double center_longitude = 0) const {
    if (abs(latitude_) >= 90 * kLatLongMultiplier) {
      VLOG(2) << "Warning: attempt to convert north/south pole to Mercator.";
      return make_pair(0, 1e+300);
    }
    pair<double, double> result;
    result.first = get_radian_longitude() - center_longitude;
    result.second = log(tan(PI / 4 + get_radian_latitude() / 2));
    return result;
  }

  // from Mercator to LatLong
  inline void FromMercator(const pair<double, double>& mercator,
                           double center_longitude = 0) {
    set_radian_latlong(2 * atan(exp(mercator.second)) - PI / 2,
                       mercator.first + center_longitude);
  }

  inline static double GMapPixelToMercator_X(double pixels, int zoom) {
    return (kGMapPixelRatio_X * pixels / (1 << zoom));
  }

  inline static double GMapPixelToMercator_Y(double pixels, int zoom) {
    return (kGMapPixelRatio_Y * pixels / (1 << zoom));
  }

  inline static double MercatorToGMapPixel_X(double mercator, int zoom) {
    return (mercator * (1 << zoom) / kGMapPixelRatio_X);
  }

  inline static double MercatorToGMapPixel_Y(double mercator, int zoom) {
    return (mercator * (1 << zoom) / kGMapPixelRatio_Y);
  }

 protected:
  int latitude_ = INFINITY_INT;   // latitude value * gLatLongMultiplier
  int longitude_ = -INFINITY_INT;  // longitude value * gLatLongMultiplier

 public:
  SERIALIZE(latitude_*1 / longitude_*2);
};

#endif  // _PUBLIC_UTIL_GEO_LATLONG_H_
