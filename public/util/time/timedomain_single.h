#ifndef _PUBLIC_UTIL_TIME_TIMEDOMAIN_SINGLE_H_
#define _PUBLIC_UTIL_TIME_TIMEDOMAIN_SINGLE_H_

#include "base/common.h"
#include "util/time/localtime.h"
#include "util/time/timeduration.h"
#include "util/time/timepoint.h"

//
// (see comments in timedomain.h for details)
//
// TimeDomainSingle has two formats, as defined below (in two derived classes)
//

template<class PointType>
class TimeDomainSingle {
 public:
  TimeDomainSingle() {};
  virtual ~TimeDomainSingle() {};

  // parse a string and create a new object
  // (caller is responsible for deleting the object after use)
  // - return pointer to new object, if successful, or NULL if failed
  // - upon return, endptr points to the first unparsed character, and
  //   error_msg contains the error message, in case of failure
  static TimeDomainSingle *Create(const char *start,
                                  const char **endptr, string *error_msg);

  // given a time (as LocalTime), check if the time is inside this
  // domain (a domain includes start time but excludes end time).
  // If inside: return true;
  //            set prev_boundary to the domain's start time
  //              (if it does not exist, set it to LocalTime::InfinitePast())
  //            set next_boundary to the domain's end time
  //              (if it does not exist, set it to LocalTime::InfiniteFuture())
  // If outside: return false;
  //           set prev_boundary to the last domain's end time
  //              (if it does not exist, set it to LocalTime::InfinitePast())
  //           set next_boundary to the next domain's start time
  //              (if it does not exist, set it to LocalTime::InfiniteFuture())
  //
  // In all cases, prev_boundary is guaranteed to be <= current
  //               next_boundary is guaranteed to be > current
  virtual bool CheckTime(const LocalTime& current,
                         const typename PointType::tAdditionalData *data,
                         LocalTime *prev_boundary,
                         LocalTime *next_boundary) const = 0;

  // for debug: return human-redable string describing this object
  virtual string Print() const = 0;
  virtual string PrintShort() const = 0;

 protected:

  // helper function for CheckTime()
  bool CheckDomain(const LocalTime& current,
                   const typename PointType::tAdditionalData *data,
                   bool found_prev_domain,
                   const LocalTime& prev_domain_start,
                   const LocalTime& prev_domain_end,
                   LocalTime *prev_boundary,
                   LocalTime *next_boundary) const {
    if (!found_prev_domain) {
      // there is no previous domain start time, so we're certainly
      // outside any domain

      if (prev_boundary)
        *prev_boundary = LocalTime::InfinitePast();

      // find the next domain boundary
      if (next_boundary)
        start_time_.GetNext(current.GetNextSecond(), data, next_boundary);

      return false;  // we are outside
    }
    else {
      // We've found a previous domain start time.  Let's check if we are
      // inside this domain

      if (current >= prev_domain_start && current < prev_domain_end) {
        // yes, we are inside this domain
        if (prev_boundary)
          *prev_boundary = prev_domain_start;
        if (next_boundary)
          *next_boundary = prev_domain_end;
        return true;  // we are inside
      }
      else {
        // no, we are outside this domain
        if (prev_boundary)
          *prev_boundary = prev_domain_end;  // previous domain's end time

        // find the next domain boundary
        if (next_boundary)
          start_time_.GetNext(current.GetNextSecond(), data, next_boundary);

        return false;  // we are outside
      }
    }
  }

  TimePoint<PointType> start_time_;
};


//
// format 1:  [(TimePoint)(TimeDuration)]
//
template<class PointType>
class TimeDomainSingle_PointDuration : public TimeDomainSingle<PointType> {
 public:
  TimeDomainSingle_PointDuration(const TimePoint<PointType>& start_time,
                                 const TimeDuration& duration) {
    TimeDomainSingle<PointType>::start_time_ = start_time;
    duration_ = duration;
  };
  virtual ~TimeDomainSingle_PointDuration() {};

  // see comments in base class TimeDomainSingle
  virtual bool CheckTime(const LocalTime& current,
                         const typename PointType::tAdditionalData *data,
                         LocalTime *prev_boundary,
                         LocalTime *next_boundary) const {
    VLOG(5) << "Calling CheckTime(" << current.Print()
           << " . " << PrintShort() << ")";

    // first, look for the previous domain's start time and end time
    // (which may or may not include current time)
    LocalTime prev_domain_start, prev_domain_end;
    bool found_prev_domain = false;

    if (TimeDomainSingle<PointType>::start_time_
        .GetPrevious(current, data, &prev_domain_start)) {
      prev_domain_end = prev_domain_start + duration_;
      found_prev_domain = true;
    }

    // next, check the relationship between current time and nearby domains
    bool in_domain = TimeDomainSingle<PointType>::CheckDomain
                       (current, data,
                        found_prev_domain, prev_domain_start, prev_domain_end,
                        prev_boundary, next_boundary);

    VLOG(3) << "CheckTime(" << current.Print()
           << " . " << PrintShort() << ") = "
           << (in_domain ? "in_domain" : "not_in_domain");

    return in_domain;
  }

  // for debug: return human-redable string describing this object
  virtual string Print() const {
    stringstream ss;
    ss << "starting at " << TimeDomainSingle<PointType>::start_time_.Print()
       << ", lasting " << duration_.Print();
    return ss.str();
  }
  virtual string PrintShort() const {
    stringstream ss;
    ss << "[(" << TimeDomainSingle<PointType>::start_time_.PrintShort()
       << "){" << duration_.PrintShort() << "}]";
    return ss.str();
  }

 protected:
  TimeDuration duration_;
};


//
// format 2:  [(TimePoint)(TimePoint)]
//
template<class PointType>
class TimeDomainSingle_TwoPoints : public TimeDomainSingle<PointType> {
 public:
  TimeDomainSingle_TwoPoints(const TimePoint<PointType>& start_time,
                             const TimePoint<PointType>& end_time) {
    TimeDomainSingle<PointType>::start_time_ = start_time;
    end_time_ = end_time;
  };
  virtual ~TimeDomainSingle_TwoPoints() {};

  // see comments in base class TimeDomainSingle
  virtual bool CheckTime(const LocalTime& current,
                         const typename PointType::tAdditionalData *data,
                         LocalTime *prev_boundary,
                         LocalTime *next_boundary) const {
    VLOG(5) << "Calling CheckTime(" << current.Print()
           << " . " << PrintShort() << ")";

    // first, look for the previous domain's start time and end time
    // (which may or may not include current time)
    LocalTime prev_domain_start, prev_domain_end;
    bool found_prev_domain = false;

    if (TimeDomainSingle<PointType>::start_time_
        .GetPrevious(current, data, &prev_domain_start))
      if (end_time_.GetNext(prev_domain_start.GetNextSecond(), data,
                            &prev_domain_end))
        found_prev_domain = true;

    // next, check the relationship between current time and nearby domains
    bool in_domain = TimeDomainSingle<PointType>::CheckDomain
                       (current, data,
                        found_prev_domain, prev_domain_start, prev_domain_end,
                        prev_boundary, next_boundary);

    VLOG(5) << "CheckTime(" << current.Print()
           << " . " << PrintShort() << ") = "
           << (in_domain ? "in_domain" : "not_in_domain");

    return in_domain;
  }

  // for debug: return human-redable string describing this object
  virtual string Print() const {
    stringstream ss;
    ss << "between " << TimeDomainSingle<PointType>::start_time_.Print()
       << " and " << end_time_.Print();
    return ss.str();
  }
  virtual string PrintShort() const {
    stringstream ss;
    ss << "[(" << TimeDomainSingle<PointType>::start_time_.PrintShort()
       << ")(" << end_time_.PrintShort() << ")]";
    return ss.str();
  }

 protected:
  TimePoint<PointType> end_time_;
};




// parse a string and create a new object
// (caller is responsible for deleting the object after use)
// - return pointer to new object, if successful, or NULL if failed
// - upon return, endptr points to the first unparsed character, and
//   error_msg contains the error message, in case of failure
template<class PointType>
TimeDomainSingle<PointType> *
  TimeDomainSingle<PointType>::Create(const char *start,
                                      const char **endptr, string *error_msg) {
  // input string format is one of the following:
  //   [(TimePoint){TimeDuration}]
  //   [(TimePoint)(TimePoint)]

  TimePoint<PointType> start_time;

  error_msg->clear();
  const char *s = strutil::SkipSpaces(start);
  // input must start with '['
  if (*s != '[') {
    *error_msg = "'[' expected";
    *endptr = s;
    return NULL;
  }
  s++;

  // next, parse (TimePoint)

  // TimePoint must start with '('
  if (*s != '(') {
    *error_msg = "'(' expected";
    *endptr = s;
    return NULL;
  }
  s++;

  if (!(start_time.Init(s, endptr, error_msg)))
    return NULL;
  s = *endptr;

  // TimePoint must end with ')'
  if (*s != ')') {
    *error_msg = "')' expected";
    *endptr = s;
    return NULL;
  }
  s++;

  TimeDomainSingle<PointType> *new_object = NULL;

  if (*s == '(') {
    // we see '(', so attempt to parse format 2 (with another TimePoint)
    TimePoint<PointType> end_time;

    if (!(end_time.Init(s + 1, endptr, error_msg)))
      return NULL;
    s = *endptr;

    // expect a ')' here
    if (*s != ')') {
      *error_msg = "')' expected";
      *endptr = s;
      return NULL;
    }
    s++;

    new_object =
      new TimeDomainSingle_TwoPoints<PointType>(start_time, end_time);
  }
  else if (*s == '{') {
    // we see '{', so attempt to parse format 1 (with {TimeDuration})
    TimeDuration duration;

    if (!(duration.Init(s + 1, endptr, error_msg)))
      return NULL;
    s = *endptr;

    // expect a closing '}' here
    if (*s != '}') {
      *error_msg = "'}' expected";
      *endptr = s;
      return NULL;
    }
    s++;

    new_object =
      new TimeDomainSingle_PointDuration<PointType>(start_time, duration);
  }
  else {
    *error_msg = "'(' or '{' expected";
    *endptr = s;
    return NULL;
  }

  // expect a closing ']' character here
  if (*s != ']') {
    *error_msg = "']' expected";
    *endptr = s;
    delete new_object;  // need to delete the object we just created
    return NULL;
  }

  ASSERT(error_msg->empty());
  *endptr = s + 1;  // done!

  return new_object;
}

#endif  // _PUBLIC_UTIL_TIME_TIMEDOMAIN_SINGLE_H_
