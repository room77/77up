#ifndef _PUBLIC_UTIL_TIME_TIMEDOMAIN_H_
#define _PUBLIC_UTIL_TIME_TIMEDOMAIN_H_

#include "base/common.h"
#include "util/time/timedomain_single.h"
#include <time.h>

//
// TimeDomain supports GDF 4.0 Time Domain representation (all sharp time
// terms, plus some fuzzy time terms such as dawn and dusk).  It includes
// the following classes:
//
//   SharpPoint: describes a specific point in time, possibly recurring
//                    Examples: m1f12          -- first Monday of January
//                              y2008m2d10h15  -- February 10, 2008 at 3pm
//
//   GenericPoint: similar to SharpPoint, but allows some fuzzy
//                      terms such as sunrise/sunset
//                      Examples: z1    -- sunrise (any day)
//                                t1z2  -- sunset on a Sunday
//                    (GenericPoint depends on timezone/latlong info)
//
//   TimeDuration: describes a specific time interval
//                 Examples: m1    -- one month
//                           h2m30 -- two and a half hours
//                           -m5   -- 5 minutes (counted in reverse)
//
//   TimePoint<T>:  (where T can be SharpPoint or GenericPoint)
//                  describes a specific point in time, given in terms of
//                  the underlying BasicTime type with an optional offset
//                  of TimeDuration, in one of the following formats:
//                    BasicTime
//                    BasicTime+TimeDuration
//                    BasicTime-TimeDuration
//               Examples: z2-h1  -- one hour before sunset
//                         z1+h2  -- two hours after sunrise
//                         h9     -- 9am
//
//   TimeDomainSingle<T>: (where T can be SharpPoint or GenericPoint)
//                        a time domain specification, in one of the following
//                        formats:
//                       [(TimePoint){TimeDuration}]
//                       [(TimePoint)(TimePoint)]
//                     (Note the difference between () and {} )
//                     The first one corresponds to GDF 4.0 spec, with a
//                     starting time and a duration.  The second one uses
//                     two TimePoint descriptions to represent an internal.
//                     Examples: [(h21){h2}]  -- 9pm to 11pm every day
//                               [(h21)(h2)]  -- 9pm to 2am (past midnight)
//                               [(z1)(z2)]   -- sunrise to sunset every day
//
//   TimeDomain<T>: (where T can be SharpPoint or GenericPoint)
//                  a complex time domain specification, with one or more
//                  TimeDomainSingle specifications combined with set operators
//                  + (union), * (intersection) or - (subtraction).
//                  Examples: [[(h21){h2}] * [[(t1){d1}] + [(t7){d1}]]]
//                                 -- 9pm to 11pm on Saturdays and Sundays
//                            [[(h8){h9}] - [(h12){h1}]]
//                                 -- 8am to 5pm, except 12 noon to 1pm
//
//
// Note that objects involving GenericPoint would depend on timezone
// and latlong, and support time arithmetic on TimePlace object.
//
// TimePlace and timezone depend on TimeDomain<SharpPoint>.
//
// Objects involving SharpPoint does not depend on timezone/latlong,
// and support time arithmetic on LocalTime.
//
//
// List of time terms supported (case-sensitive):
//   SharpPoint:
//     ynnnn -- year nnnn
//     Mnn   -- month nn (1 to 12)   *** note: different from struct tm
//     wnn   -- week nn within a year (1 to 53)
//     dnn   -- day nn within a month (1 to 28, 29, 30 or 31)
//     tn    -- day n within a week (1: Sunday; 2: Monday; ...; 7: Saturday)
//                                   *** note: different from struct tm
//     fxn   -- n-th day of the x-th week within a month
//                (example: f21 -- second Sunday of a month)
//     lxn   -- n-th day of the x-th week counted in reverse, within a month
//                (example: l12 -- last Monday of a month)
//     hnn   -- hour nn within a day (0 to 23)
//     mnn   -- minute nn within an hour (0 to 59)
//     snn   -- second nn within a second (0 to 59)
//
//   GenericPoint:  everything above, plus
//     z1    -- sunrise
//     z2    -- sunset
//
//   duration:
//     ynn   -- nn years    (y1 == M12)
//     Mnn   -- nn months   (a month can be 28, 29, 30 or 31 days)
//     wnn   -- nn weeks    (w1 == d7)
//     dnn   -- nn days     (d1 == h24)
//     hnn   -- nn hours    (h1 == m60)
//     mnn   -- nn minutes  (m1 == s60)
//     snn   -- nn seconds


template<class PointType>     // PointType can be SharpPoint or GenericPoint
class TimeDomain {
 public:
  // construct an empty tree
  TimeDomain() { root_ = NULL; };
  // copy constructor
  TimeDomain(const TimeDomain& other) { root_ = NULL;  CopyFrom(other); }
  // initialize from time domain string
  TimeDomain(const string& domain_str) {
    root_ = NULL;
    string error_msg;
    ASSERT(Init(domain_str, &error_msg))
      << "Error parsing " << domain_str << ": " << error_msg;
  }
  ~TimeDomain() { Clear(); };

  // assignment operator
  inline TimeDomain& operator=(const TimeDomain& other) {
    CopyFrom(other);
    return *this;
  }

  // copy content from another TimeDomain object
  void CopyFrom(const TimeDomain& other) {
    Clear();
    // dump content to string and parse it again
    string other_str = other.PrintShort();
    string error_msg;
    ASSERT(Init(other_str, &error_msg))
      << "In copy constructor: unable to parse " << other_str
      << ": " << error_msg;;
  }

  inline void Clear() {
    Destroy(root_);
    root_ = NULL;
  }

  typedef enum {
    TD_NONE, TD_UNION, TD_INTERSECTION, TD_SUBTRACTION
  } tTimeDomain_TreeOp;

  typedef struct _td_tree {
    tTimeDomain_TreeOp td_operator;
    TimeDomainSingle<PointType> *element;  // NULL for all non-leaf nodes
    struct _td_tree *first_operand, *second_operand;  // NULL for leaf nodes
  } tTimeDomain_TreeElement;

  void Destroy(tTimeDomain_TreeElement *tree) {
    if (tree != NULL) {
      Destroy(tree->first_operand);
      Destroy(tree->second_operand);
      delete tree->element;
      delete tree;
    }
  }

  // initialize from a string
  // (return true if successful; upon return, endptr points to the first
  //  unparsed character)
  inline bool Init(const char *start, const char **endptr, string *error_msg) {
    error_msg->clear();
    root_ = ParseExp(start, endptr, error_msg);
    return (error_msg->empty());
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

  // get the next time domain (start and end times) after the current time
  bool GetNextDomain(const LocalTime& current,
                     const typename PointType::tAdditionalData *data,
                     LocalTime *next_start,
                     LocalTime *next_end) const {
    LocalTime outside = current;
    // make sure "outside" is truly outside --
    //   if current time is inside a domain, reset "outside" to its end point
    LocalTime prev, next;
    bool in_domain = CheckTime(current, data, &prev, &next);
    if (in_domain) {
      outside = next;
      // Now "outside" is outside any domain.
      // Next, find the next domain's starting point
      if (outside.IsInfinite())
        return false;  // unable to find a domain
      ASSERT(CheckTime(outside, data, &prev, next_start) == false);
    }
    else
      *next_start = next;

    if (next_start->IsInfinite())
      return false;

    ASSERT(CheckTime(*next_start, data, &prev, next_end));

    if (next_end->IsInfinite())
      return false;
    return true;
  }

  // given a time (as LocalTime), check if the time is inside this domain
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
  bool CheckTime(const LocalTime& current,
                 const typename PointType::tAdditionalData *data,
                 LocalTime *prev_boundary,
                 LocalTime *next_boundary) const;

  // return the earlist time (at or after current time) that is inside the
  // time domain
  inline LocalTime ForwardIntoDomain(const LocalTime& current,
                                     const typename PointType::tAdditionalData
                                       *data) {
    LocalTime next;
    bool in_domain = CheckTime(current, data, NULL, &next);
    return (in_domain ? current : next);
  }

  // return the latest time (at or before current time) that is inside the
  // time domain
  inline LocalTime ReverseIntoDomain(const LocalTime& current,
                                     const typename PointType::tAdditionalData
                                       *data) {
    LocalTime prev;
    bool in_domain = CheckTime(current, data, &prev, NULL);
    return (in_domain ? current : (prev.GetPrevSecond()));
  }

  // used by CheckTime -- check a subtree's time domain info
  //   and suggests boundary times for further testing
  bool CheckTime_Subtree(tTimeDomain_TreeElement *subtree,
                         const LocalTime& current,
                         const typename PointType::tAdditionalData *data,
                         LocalTime *prev_boundary_trial,
                         LocalTime *next_boundary_trial) const;

  // for debug: return human-redable string describing this object
  inline string Print() const {
    return PrintSubTree(root_, true);
  }
  inline string PrintShort() const {
    return PrintSubTree(root_, false);
  }
  inline string PrintSubTree(tTimeDomain_TreeElement *tree,
                             bool verbose) const {
    if (!tree)
      return "";
    switch (tree->td_operator) {
    case TD_NONE: {
      // leaf node
      return (verbose ? tree->element->Print() : tree->element->PrintShort());
    }
    case TD_UNION: {
      stringstream ss;
      ss << "[" << PrintSubTree(tree->first_operand, verbose) << " + "
         << PrintSubTree(tree->second_operand, verbose) << "]";
      return ss.str();
    }
    case TD_INTERSECTION: {
      stringstream ss;
      ss << "[" << PrintSubTree(tree->first_operand, verbose) << " * "
         << PrintSubTree(tree->second_operand, verbose) << "]";
      return ss.str();
    }
    case TD_SUBTRACTION: {
      stringstream ss;
      ss << "[" << PrintSubTree(tree->first_operand, verbose) << " - "
         << PrintSubTree(tree->second_operand, verbose) << "]";
      return ss.str();
    }
    default:
      ASSERT(false);
    }
    return "";
  }

 protected:
  // parse one TimeDomain expression
  //    expression = term +/- term +/- ... +/- term
  //    term = factor * factor * ... * factor
  //    factor = TimeDomainSingle | [expression]

  // ParseExp:
  //    expression = term +/- term +/- ... +/- term
  tTimeDomain_TreeElement *ParseExp(const char *start,
                                    const char **endptr,
                                    string *error_msg) {
    tTimeDomain_TreeElement *first_term = ParseTerm(start, endptr, error_msg);

    if (!first_term) return NULL;

    const char *s = strutil::SkipSpaces(*endptr);
    while (*s == '+' || *s == '-') {
      bool is_union = (*s == '+');
      tTimeDomain_TreeElement *second_term =
        ParseTerm(s + 1, endptr, error_msg);

      if (second_term == NULL) {
        Destroy(first_term);
        if (error_msg->empty())
          *error_msg = "Timedomain expression expected";
        return NULL;
      }

      // construct a tree element using the two terms
      tTimeDomain_TreeElement *new_element = new tTimeDomain_TreeElement;
      new_element->td_operator = (is_union ? TD_UNION : TD_SUBTRACTION);
      new_element->element = NULL;
      new_element->first_operand = first_term;
      new_element->second_operand = second_term;

      // now the combined element becomes the first term
      first_term = new_element;

      // continue parsing
      s = strutil::SkipSpaces(*endptr);
    }

    // now first_term points to a subtree for this expression
    return first_term;
  }

  // ParseTerm:
  //    term = factor * factor * ... * factor
  tTimeDomain_TreeElement *ParseTerm(const char *start,
                                     const char **endptr,
                                     string *error_msg) {
    tTimeDomain_TreeElement *first_factor =
      ParseFactor(start, endptr, error_msg);

    if (!first_factor) return NULL;

    const char *s = strutil::SkipSpaces(*endptr);
    while (*s == '*') {
      tTimeDomain_TreeElement *second_factor =
        ParseFactor(s + 1, endptr, error_msg);

      if (second_factor == NULL) {
        Destroy(first_factor);
        if (error_msg->empty())
          *error_msg = "Timedomain expression expected";
        return NULL;
      }

      // construct a tree element using the two factors
      tTimeDomain_TreeElement *new_element = new tTimeDomain_TreeElement;
      new_element->td_operator = TD_INTERSECTION;
      new_element->element = NULL;
      new_element->first_operand = first_factor;
      new_element->second_operand = second_factor;

      // now the combined element becomes the first factor
      first_factor = new_element;

      // continue parsing
      s = strutil::SkipSpaces(*endptr);
    }

    // now first_factor points to a subtree for this expression
    return first_factor;
  }

  // ParseFactor:
  //    factor = TimeDomainSingle | [expression]
  tTimeDomain_TreeElement *ParseFactor(const char *start,
                                       const char **endptr,
                                       string *error_msg) {
    const char *s = strutil::SkipSpaces(start);
    *endptr = s;

    if (*s != '[')
      return NULL;   // end of recognizable content
    else {
      if (*(s + 1) == '[') {
        // we have a nested TimeDomain expression here
        tTimeDomain_TreeElement *nested = ParseExp(s + 1, endptr, error_msg);
        if (nested == NULL && !error_msg->empty())  // an error occurred
          return NULL;
        s = strutil::SkipSpaces(*endptr);
        if (*s != ']') {
          // syntax error
          Destroy(nested);
          *error_msg = "']' expected";
          *endptr = s;
          return NULL;
        }
        *endptr = s + 1;
        return nested;
      }
      else {
        // we have a TimeDomainSingle expression here
        TimeDomainSingle<PointType> *single
          = TimeDomainSingle<PointType>::Create(s, endptr, error_msg);
        if (single == NULL)  // an error occurred
          return NULL;

        tTimeDomain_TreeElement *new_element = new tTimeDomain_TreeElement;
        new_element->td_operator = TD_NONE;
        new_element->element = single;
        new_element->first_operand = NULL;
        new_element->second_operand = NULL;
        return new_element;
      }
    }
  }

  tTimeDomain_TreeElement *root_;
};



template<class PointType>
bool TimeDomain<PointType>::CheckTime
       (const LocalTime& current,
        const typename PointType::tAdditionalData *data,
        LocalTime *prev_boundary,
        LocalTime *next_boundary) const {
  VLOG(5) << "Calling CheckTime(" << current.Print()
         << " . " << PrintShort() << ")";

  LocalTime prev_boundary_trial, next_boundary_trial;

  bool in_domain =
    CheckTime_Subtree(root_, current, data,
                      (prev_boundary ? &prev_boundary_trial : NULL),
                      (next_boundary ? &next_boundary_trial : NULL));

  VLOG(5) << "CheckTime(" << current.Print()
         << " . " << PrintShort() << ") = "
         << (in_domain ? "in_domain" : "not_in_domain");

  //
  // use CheckTime_Subtree repeatedly to find the previous/next true boundary
  // for this domain (CheckTime_Subtree gives a hint on the boundary of one
  // of the domain's components, but it is not necessarily the true boundary
  // of the entire domain expression)
  //

  // find the previous boundary
  if (prev_boundary) {
    *prev_boundary = LocalTime::InfinitePast();

    // try up to 50 iterations
    for (int i = 0; prev_boundary_trial.IsFinite() && i < 50; i++) {
      LocalTime earlier_boundary;

      bool d = CheckTime_Subtree(root_,
                                 prev_boundary_trial.GetPrevSecond(), data,
                                 &earlier_boundary, NULL);
      VLOG(5) << "Try #" << i + 1 << " (earlier) for "
             << PrintSubTree(root_, false)
             << ": " << prev_boundary_trial.Print()
             << " -> " << earlier_boundary.Print();
      if (d != in_domain) {
        // success!
        *prev_boundary = prev_boundary_trial;
        break;
      }

      prev_boundary_trial = earlier_boundary;

      if (i == 49)
        VLOG(2) << "Unable to find previous boundary: current = "
               << current.Print() << ", constraint is " << PrintShort();
    }
  }

  // find the next boundary
  if (next_boundary) {
    *next_boundary = LocalTime::InfiniteFuture();

    // try up to 50 iterations
    for (int i = 0; next_boundary_trial.IsFinite() && i < 50; i++) {
      LocalTime later_boundary;

      bool d = CheckTime_Subtree(root_,
                                 next_boundary_trial, data,
                                 NULL, &later_boundary);
      VLOG(5) << "Try #" << i + 1 << " (later) for "
             << PrintSubTree(root_, false)
             << ": " << next_boundary_trial.Print()
             << " -> " << later_boundary.Print();
      if (d != in_domain) {
        // success!
        *next_boundary = next_boundary_trial;
        break;
      }

      next_boundary_trial = later_boundary;

      if (i == 49)
        VLOG(2) << "Unable to find next boundary: current = "
               << current.Print() << ", constraint is " << PrintShort();
    }
  }

  return in_domain;
}


template<class PointType>
bool TimeDomain<PointType>::CheckTime_Subtree
         (tTimeDomain_TreeElement *subtree,
          const LocalTime& current,
          const typename PointType::tAdditionalData *data,
          LocalTime *prev_boundary_trial,
          LocalTime *next_boundary_trial) const {
  if (subtree == NULL) {
    // empty domain
    if (prev_boundary_trial)
      *prev_boundary_trial = LocalTime::InfinitePast();
    if (next_boundary_trial)
      *next_boundary_trial = LocalTime::InfiniteFuture();
    return false;
  }

  VLOG(5) << "Calling CheckTime_Subtree(" << current.Print()
         << " . " << PrintSubTree(subtree, false) << ")";

  if (subtree->td_operator == TD_NONE) {
    // this is a leaf node
    ASSERT(subtree->first_operand == NULL);
    ASSERT(subtree->second_operand == NULL);
    ASSERT(subtree->element != NULL);
    return subtree->element->CheckTime(current, data,
                                       prev_boundary_trial,
                                       next_boundary_trial);
  }

  // this is a non-leaf node
  ASSERT(subtree->element == NULL);
  ASSERT(subtree->first_operand != NULL);
  ASSERT(subtree->second_operand != NULL);

  LocalTime prev1, next1, prev2, next2;
  bool in_domain1 =
    TimeDomain<PointType>::
    CheckTime_Subtree(subtree->first_operand,
                      current, data,
                      (prev_boundary_trial ? &prev1 : NULL),
                      (next_boundary_trial ? &next1 : NULL));
  bool in_domain2 =
    TimeDomain<PointType>::
    CheckTime_Subtree(subtree->second_operand,
                      current, data,
                      (prev_boundary_trial ? &prev2 : NULL),
                      (next_boundary_trial ? &next2 : NULL));
  switch (subtree->td_operator) {
    case TD_UNION: {
      if (in_domain1) {
        if (in_domain2) {
          // in both domain 1 and domain 2

          // use the earlier of two prev_boundaries, and
          // the later of two next_boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = min(prev1, prev2);
          if (next_boundary_trial)
            *next_boundary_trial = max(next1, next2);
        }
        else {
          // in domain 1, not in domain 2

          // use domain 1's boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = prev1;
          if (next_boundary_trial)
            *next_boundary_trial = next1;
        }
      }
      else {
        if (in_domain2) {
          // in domain 2, not in domain 1

          // use domain 2's boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = prev2;
          if (next_boundary_trial)
            *next_boundary_trial = next2;
        }
        else {
          // in neither domain 1 nor domain 2

          // use the later of two prev_boundaries, and
          // the earlier of two next_boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = max(prev1, prev2);
          if (next_boundary_trial)
            *next_boundary_trial = min(next1, next2);
        }
      }
      return (in_domain1 || in_domain2);
    }
    case TD_SUBTRACTION:
      in_domain2 = !in_domain2;
      // a - b is equivalent to a * ~b, so we reverse in_domain2 here
      // and let TD_INTERSECTION handle the rest

      // (Note: there is no "break" or "return" here, because we want the
      //  control flow to go into TD_INTERSECTION)

    case TD_INTERSECTION: {
      if (in_domain1) {
        if (in_domain2) {
          // in both domain 1 and domain 2

          // use the later of two prev_boundaries, and
          // the earlier of two next_boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = max(prev1, prev2);
          if (next_boundary_trial)
            *next_boundary_trial = min(next1, next2);
        }
        else {
          // in domain 1, not in domain 2

          // use domain 2's boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = prev2;
          if (next_boundary_trial)
            *next_boundary_trial = next2;
        }
      }
      else {
        if (in_domain2) {
          // in domain 2, not in domain 1

          // use domain 1's boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = prev1;
          if (next_boundary_trial)
            *next_boundary_trial = next1;
        }
        else {
          // in neither domain 1 nor domain 2

          // use the earlier of two prev_boundaries, and
          // the later of two next_boundaries
          if (prev_boundary_trial)
            *prev_boundary_trial = min(prev1, prev2);
          if (next_boundary_trial)
            *next_boundary_trial = max(next1, next2);
        }
      }

      return (in_domain1 && in_domain2);
    }
    default: break;
  }

  ASSERT(false);
  return false;
}


#endif  // _PUBLIC_UTIL_TIME_TIMEDOMAIN_H_
