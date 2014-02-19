#include "util/geo/latlong.h"

// Calculate the surface distance of two points, assuming the earth is
// a sphere with radius EARTH_RADIUS
// (result is in kilometers)
double LatLong::SurfaceDistance(const LatLong& p1, const LatLong& p2) {
  double lat1 = p1.get_radian_latitude();
  double long1 = p1.get_radian_longitude();
  double lat2 = p2.get_radian_latitude();
  double long2 = p2.get_radian_longitude();

  if (fabs(lat1 - lat2) < 1e-15 && fabs(long1 - long2) < 1e-15)
    return 0;  // same point

  // Euclidean coordinates of the two surface points:
  // (x1, y1, z1) and (x2, y2, z2)

  //                   x1                       x2
  // double t1 = cos(lat1) * cos(long1) - cos(lat2) * cos(long2);
  ////                 y1                       y2
  // double t2 = cos(lat1) * sin(long1) - cos(lat2) * sin(long2);
  ////           z1           z2
  // double t3 = sin(lat1) - sin(lat2);

  //// square of euclidean distance between two points,
  //// assuming radius of earth = 1
  // double linear_distance_squared = t1 * t1 + t2 * t2 + t3 * t3;

  // angle between two lines connecting center of earth to two surface points
  // double b = acos(1.0 - linear_distance_squared / 2);

  // all of the above can be simplifed to:
  // acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(long1 - long2))
  double bb = (sin(lat1) * sin(lat2) +
               cos(lat1) * cos(lat2) * cos(long1 - long2));
  if (bb > 1) bb = 1;    // account for floating point inaccuracies
  if (bb < -1) bb = -1;  // account for floating point inaccuracies

  double b = acos(bb);

  if (b < 0)
    b += PI;

  // surface distace
  double distance = EARTH_RADIUS * b;
  return distance;
}

// calculate the direction from point 1 to point 2.
// result unit is in radian:
//   0      -- north
//   pi/2   -- east
//   pi     -- south
//   pi*1.5 -- west
double LatLong::RadianDirection(const LatLong& src, const LatLong& dest) {
  //
  // special cases involving north/south poles
  //
  // going to north pole -- always north
  if (dest.get_multiplied_latitude() == 90 * kLatLongMultiplier) return 0;    // north
  // going to south pole -- always south
  if (dest.get_multiplied_latitude() == -90 * kLatLongMultiplier) return PI;  // south
  // going from north pole -- always south
  if (src.get_multiplied_latitude() == 90 * kLatLongMultiplier) return PI;    // south
  // going from south pole -- always north
  if (src.get_multiplied_latitude() == -90 * kLatLongMultiplier) return 0;    // north

  double long1 = src.get_radian_longitude();
  double lat1  = src.get_radian_latitude();
  double long2 = dest.get_radian_longitude();
  double lat2  = dest.get_radian_latitude();
  double diff_long = long2 - long1;

  double t1 = sin(diff_long) * cos(lat2);
  double t2 = cos(lat1) * sin(lat2) - cos(lat2) * cos(diff_long) * sin(lat1);
  if (fabs(t1) < 1e-10 && fabs(t2) < 1e-10) {
    LOG(INFO) << "Warning: calculating direction from a point to itself, "
           << "or from a point to its direct opposite on the "
           << "other side of the earth! --- "
           << setprecision(8) << "(" << long1 << ", " << lat1
           << ") to (" << long2 << ", " << lat2 << ")\n";
    return 0;  // an error occurred; assume it's north
  }

  double direction = atan2(t1, t2);

  // Above is the formula to calculate road direction from two sets of
  // longitude/latitude values.
  //
  // Here is one way to derive it (direction from point A to point B):
  //
  // Rotate the earth so that point A has longitude 0 and point B has
  // longitude "diff_long".  Assume the earth's radius is 1.  The origin
  // (0,0,0) is the earth's center.  A and B have the following coordinates:
  //
  // A: (cos(lat1), 0, sin(lat1))
  // B: (cos(lat2)cos(diff_long), cos(lat2)sin(diff_long), sin(lat2))
  //
  // The direction vector from A towards B is A x B x A because:
  //
  // A x B (cross-product) gives a vector C perpendicular to the plane P
  // defined by A and B (which contains the great-circle).
  // A x B x A gives a vector perpendicular to A and C.
  // Any line perpendicular to C must be on plane P, so A x B x A is on
  // plane P and it's perpendicular to A (direction of the tangent to
  // earth's surface at point A).
  //
  // If we replace B with the north pole (0,0,1), then we get the "north"
  // vector.  The angle between the "north" vector and A x B x A is the
  // direction.

  if (direction < 0)
    direction += (PI * 2);
  return direction;
}

void LatLong::GetExtendedBoundingBox(const LatLong& nw, const LatLong& se,
                                     LatLong *e_nw, LatLong *e_se) {
  int diff_lat = nw.get_multiplied_latitude() - se.get_multiplied_latitude();
  int diff_long = se.get_multiplied_longitude() - nw.get_multiplied_longitude();
  if (diff_long <= 0 || diff_lat <= 0) {
    // don't change anything
    *e_nw = nw;
    *e_se = se;
  }
  else {
    // extended bounding box to all four directions
    long long e_nw_lat = nw.get_multiplied_latitude();
    e_nw_lat += diff_lat;
    long long e_nw_long = nw.get_multiplied_longitude();
    e_nw_long -= diff_long;
    long long e_se_lat = se.get_multiplied_latitude();
    e_se_lat -= diff_lat;
    long long e_se_long = se.get_multiplied_longitude();
    e_se_long += diff_long;

    if (e_nw_lat > 90 * kLatLongMultiplier)
      e_nw_lat = 90 * kLatLongMultiplier;
    if (e_nw_long < -180 * kLatLongMultiplier)
      e_nw_long = -180 * kLatLongMultiplier;
    if (e_se_lat < -90 * kLatLongMultiplier)
      e_se_lat = -90 * kLatLongMultiplier;
    if (e_se_long > 180 * kLatLongMultiplier)
      e_se_long = 180 * kLatLongMultiplier;
    e_nw->set_multiplied_latlong(e_nw_lat, e_nw_long);
    e_se->set_multiplied_latlong(e_se_lat, e_se_long);
  }
}

// start from the given (northwest, southeast) pair, expand it if necessary
// to accomodate another bounding box (new_nw, new_se)
// (note: we ignore cases where the bounding box crosses 180-degree longitude)
void LatLong::ExpandBoundingBox(const pair<double, double>& new_nw,
                                const pair<double, double>& new_se,
                                pair<double, double> *northwest,
                                pair<double, double> *southeast) {
  if (new_nw.first > northwest->first)
    northwest->first = new_nw.first;
  if (new_nw.second < northwest->second)
    northwest->second = new_nw.second;
  if (new_se.first < southeast->first)
    southeast->first = new_se.first;
  if (new_se.second > southeast->second)
    southeast->second = new_se.second;
}

