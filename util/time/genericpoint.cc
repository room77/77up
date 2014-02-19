#include "util/time/genericpoint.h"

// initialize from a string
// (return true if successful; upon return, endptr points to the first
//  unparsed character)
bool GenericPoint::Init(const char *start, const char **endptr,
                        string *error_msg) {
  error_msg->clear();
  const char *first_char = strutil::SkipSpaces(start);

  if (SharpPoint::InitFromSpec(first_char, endptr, error_msg)) {
    *endptr = ParseNext(*endptr, 'z', 1, 2, &fuzzy_type_, error_msg);
    if (!error_msg->empty()) return false;
    if (fuzzy_type_ != UNDEFINED) {
      if (hour_ != UNDEFINED ||
          minute_ != UNDEFINED || second_ != UNDEFINED) {
        *error_msg =
          "hour/minute/second cannot be combined with "
          "fuzzy timepoint specifications";
        return false;
      }
    }
    else {
      // no fuzzy spec; try to set default values for SharpPoint
      SharpPoint::SetDefaultValues();
    }
    is_pure_fuzzy_ = (fuzzy_type_ != UNDEFINED && SharpPoint::AllEmpty());
    return true;
  }
  else
    return false;
}

// given a time, find the next time point that matches this specification
// -- return true if successful ("next" is set)
// -- return false if such a time cannot be found
bool GenericPoint::GetNext(const LocalTime& current, const Place *place,
                           LocalTime *next) const {
  Timezone tz = place->get_tz();
  LocalTime base = current;
  for (int i = 0; i < 2; i++) {
    LocalTime ref;
    bool exists = SharpPoint::GetNext(base, NULL, &ref);
    if (!exists) {
      *next = LocalTime::InfiniteFuture();
      return false;
    }

    switch (fuzzy_type_) {
    case UNDEFINED: {
      *next = ref;
      return exists;
    }
    case FUZZY_SUNRISE: {
      *next = Nature::NextSunrise(Time::GetUniversalTime(tz, ref),
                                  *place).GetLocalTime(tz);
      if (!(next->IsFinite()) || is_pure_fuzzy_ || next->SameDay(ref))
        return next->IsFinite();
      else {
        // not pure fuzzy, and NextSunrise results in another day.
        // let's try twice (break out of switch statement and let
        // "for" loop take care of another trial)
        base = *next;
        break;
      }
    }
    case FUZZY_SUNSET: {
      *next = Nature::NextSunset(Time::GetUniversalTime(tz, ref),
                                 *place).GetLocalTime(tz);
      if (!(next->IsFinite()) || is_pure_fuzzy_ || next->SameDay(ref))
        return next->IsFinite();
      else {
        // not pure fuzzy, and NextSunset results in another day.
        // let's try twice (break out of switch statement and let
        // "for" loop take care of another trial)
        base = *next;
        break;
      }
    }
    default:
      ASSERT(false) << "Invalid fuzzy_type";
    }
  }
  // if control reaches here, we are unable to find sunset/sunrise for this day
  *next = LocalTime::InfiniteFuture();
  return false;
}

// given a time, return the previous time point that matches this
// specification
bool GenericPoint::GetPrevious(const LocalTime& current, const Place *place,
                               LocalTime *previous) const {
  Timezone tz = place->get_tz();
  LocalTime base = current;
  for (int i = 0; i < 2; i++) {
    LocalTime ref;
    bool exists = SharpPoint::GetPrevious(base, NULL, &ref);
    if (!exists) {
      *previous = LocalTime::InfinitePast();
      return false;
    }

    switch (fuzzy_type_) {
    case UNDEFINED: {
      *previous = ref;
      return exists;
    }
    case FUZZY_SUNRISE: {
      *previous = Nature::PreviousSunrise(Time::GetUniversalTime(tz, ref),
                                          *place).GetLocalTime(tz);
      if (!(previous->IsFinite()) || is_pure_fuzzy_ || previous->SameDay(ref))
        return previous->IsFinite();
      else {
        // not pure fuzzy, and PreviousSunrise results in another day.
        // let's try twice (break out of switch statement and let
        // "for" loop take care of another trial)
        base = *previous;
        break;
      }
    }
    case FUZZY_SUNSET: {
      *previous = Nature::PreviousSunset(Time::GetUniversalTime(tz, ref),
                                         *place).GetLocalTime(tz);
      if (!(previous->IsFinite()) || is_pure_fuzzy_ || previous->SameDay(ref))
        return previous->IsFinite();
      else {
        // not pure fuzzy, and PreviousSunset results in another day.
        // let's try twice (break out of switch statement and let
        // "for" loop take care of another trial)
        base = *previous;
        break;
      }
    }
    default:
      ASSERT(false) << "Invalid fuzzy_type";
    }
  }
  // if control reaches here, we are unable to find sunset/sunrise for this day
  *previous = LocalTime::InfinitePast();
  return false;
}

// return human-redable string describing this object
string GenericPoint::Print() const {
  stringstream ss;
  ss << SharpPoint::Print();
  switch (fuzzy_type_) {
  case FUZZY_SUNRISE: {
    ss << " (sunrise)";
    break;
  }
  case FUZZY_SUNSET: {
    ss << " (sunset)";
    break;
  }
  }
  return ss.str();
}

string GenericPoint::PrintShort() const {
  stringstream ss;
  ss << SharpPoint::PrintShort();
  if (fuzzy_type_ == FUZZY_SUNRISE ||
      fuzzy_type_ == FUZZY_SUNSET)
    ss << "z" << fuzzy_type_;
  return ss.str();
}

