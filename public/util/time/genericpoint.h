#ifndef _PUBLIC_UTIL_TIME_GENERICPOINT_H_
#define _PUBLIC_UTIL_TIME_GENERICPOINT_H_

//
// Part of TimeDomain family of classes.
//   (See comments of timedomain.h for details)
//

#include "base/common.h"
#include "util/geo/place.h"
#include "util/time/utime.h"
#include "util/time/sharppoint.h"
#include "util/time/nature/sun.h"

class GenericPoint : public SharpPoint {
 public:
  GenericPoint() {
    fuzzy_type_ = UNDEFINED;
    is_pure_fuzzy_ = false;
  };
  ~GenericPoint() {};

  // type of auxiliary data to be passed to GetNext() and GetPrevious()
  // -- void (no additional data needed) for SharpPoint
  // -- Place for FuzzyPoint
  typedef Place tAdditionalData;

  void Clear() {
    // set everything to undefined
    SharpPoint::Clear();
    fuzzy_type_ = UNDEFINED;
    is_pure_fuzzy_ = false;
  }

  // initialize from a string
  // (return true if successful; upon return, endptr points to the first
  //  unparsed character)
  bool Init(const char *start, const char **endptr, string *error_msg);

  // for debug: initialize from a string and check for extra characters
  //            after the end
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

  // given a time, find the next time point that matches this specification
  // -- return true if successful ("next" is set)
  // -- return false if such a time cannot be found
  bool GetNext(const LocalTime& current, const Place *place,
               LocalTime *next) const;

  // given a time, return the previous time point that matches this
  // specification
  bool GetPrevious(const LocalTime& current, const Place *place,
                   LocalTime *previous) const;

  // return human-redable string describing this object
  string Print() const;
  string PrintShort() const;

 protected:
  static const int FUZZY_SUNRISE = 1;  // sunrise within a day
  static const int FUZZY_SUNSET = 2;   // sunset within a day
  short fuzzy_type_;
  bool is_pure_fuzzy_;
};



#endif  // _PUBLIC_UTIL_TIME_GENERICPOINT_H_
