#ifndef _PUBLIC_UTIL_TIME_TIMEPOINT_H_
#define _PUBLIC_UTIL_TIME_TIMEPOINT_H_

#include "base/common.h"
#include "util/time/localtime.h"

//
// (see comments in timedomain.h for details)
//
// PointType can be either SharpPoint or GenericPoint
//
template<class PointType>
class TimePoint {
 public:
  TimePoint() {};
  // initialize from a string representation of a time point
  TimePoint(const string& timepoint_str) {
    string error_msg;
    ASSERT(Init(timepoint_str, &error_msg))
      << "Error parsing " << timepoint_str << ": " << error_msg;
  };
  ~TimePoint() {};

  // initialize from a string
  // (return true if successful; upon return, endptr points to the first
  //  unparsed character)
  bool Init(const char *start, const char **endptr, string *error_msg) {
    error_msg->clear();
    const char *s = strutil::SkipSpaces(start);
    if (!(point_.Init(s, endptr, error_msg)))
      return false;

    s = *endptr;
    if (*s == '+' || *s == '-') {
      bool negative_offset = (*s == '-');
      if (!(offset_.Init(s + 1, endptr, error_msg)))
        return false;

      s = *endptr;
      if (negative_offset)
        offset_.Reverse();
    }

    *endptr = s;
    return true;
  }

  // initialize from a string and check for extra characters after the end
  bool Init(const string& input, string *error_msg) {
    const char *end;
    string error;
    if (Init(input.c_str(), &end, &error)) {
      if (*(strutil::SkipSpaces(end)) != '\0') {
        *error_msg = strutil::ErrorMsg("extra characters at the end",
                                       end - input.c_str() + 1);
        return false;
      }
      else {
        error_msg->clear();
        return true;
      }
    }
    else {
      *error_msg = strutil::ErrorMsg(error, end - input.c_str() + 1);
      return false;
    }
  }

  // given a time (as LocalTime), find the next time point that
  // matches this specification  (must be >= current)
  // -- return true if successful ("next" is set)
  // -- return false if such a time cannot be found
  bool GetNext(const LocalTime& current,
               const typename PointType::tAdditionalData *data,
               LocalTime *next) const {
    VLOG(5) << "Calling GetNext(" << current.Print()
           << " . " << PrintShort() << ")";

    ASSERT(current.IsFinite());
    LocalTime ref = current - offset_;
    LocalTime ref_next;
    bool exists = point_.GetNext(ref, data, &ref_next);
    if (next)
      *next = (exists ? (ref_next + offset_) : LocalTime::InfiniteFuture());

    VLOG(5) << "GetNext(" << current.Print()
           << " . " << PrintShort() << ") = "
           << (next ? next->Print() : (exists ? "exists" : "does not exist"));
    return exists;
  }

  // given a time (as LocalTime), return the previous time point that
  // matches this specification  (must be <= current)
  bool GetPrevious(const LocalTime& current,
                   const typename PointType::tAdditionalData *data,
                   LocalTime *previous) const {
    VLOG(5) << "Calling GetPrevious(" << current.Print()
           << " . " << PrintShort() << ")";

    ASSERT(current.IsFinite());
    LocalTime ref = current - offset_;
    LocalTime ref_prev;
    bool exists = point_.GetPrevious(ref, data, &ref_prev);
    if (previous)
      *previous = (exists ? (ref_prev + offset_) : LocalTime::InfinitePast());

    VLOG(5) << "GetPrevious(" << current.Print()
           << " . " << PrintShort() << ") = "
           << (previous ?
               previous->Print() :
               (exists ? "exists" : "does not exist"));
    return exists;
  }

  // for debug: return human-redable string describing this object
  string Print() const {
    stringstream ss;
    ss << point_.Print();
    int offset_sign = offset_.GetSign();
    if (offset_sign < 0)
      ss << " minus " << offset_.GetReverse().Print();
    else if (offset_sign > 0)
      ss << " plus " << offset_.Print();
    return ss.str();
  }
  string PrintShort() const {
    stringstream ss;
    ss << point_.PrintShort();
    int offset_sign = offset_.GetSign();
    if (offset_sign < 0)
      ss << "-" << offset_.GetReverse().PrintShort();
    else if (offset_sign > 0)
      ss << "+" << offset_.PrintShort();
    return ss.str();
  }

 private:
  PointType point_;
  TimeDuration offset_;
};



#endif  // _PUBLIC_UTIL_TIME_TIMEPOINT_H_
