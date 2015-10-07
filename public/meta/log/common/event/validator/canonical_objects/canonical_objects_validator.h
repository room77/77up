// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_META_LOG_COMMON_EVENT_VALIDATOR_CANONICAL_OBJECTS_CANONICAL_OBJECTS_VALIDATOR_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_VALIDATOR_CANONICAL_OBJECTS_CANONICAL_OBJECTS_VALIDATOR_H_

#include <set>

#include "base/defs.h"
#include "meta/log/common/event/datatypes/canonical_objects.h"
#include "meta/log/common/event/constants.h"
#include "meta/log/common/log_datatypes.h"

using logging::event::kLatitudeMin;
using logging::event::kLatitudeMax;
using logging::event::kLongitudeMin;
using logging::event::kLongitudeMax;

namespace {

inline bool CheckLatitude(double lat) {
  return (lat >= kLatitudeMin && lat <= kLatitudeMax);
}

inline bool CheckLongitude(double lon) {
  return (lon >= kLongitudeMin && lon <= kLongitudeMax);
}

inline bool CheckRooms(int rooms) {
  return (rooms >= 1);
}

inline bool CheckGuests(int guests) {
  return (guests >= 1);
}

inline bool CheckNumNights(bool dateless, int num_nights) {
  return ((num_nights >= 1 && !dateless) || (num_nights == 0 && dateless));
}

inline bool CheckDaysUntilCheckin(int days_until_checkin) {
  return (days_until_checkin >= 0);
}

inline bool CheckStayDates(bool dateless, LocalDate checkin, LocalDate checkout) {
  return ((checkout > checkin && !dateless) || (checkout == checkin && dateless));
}

// this function checks whether r is a half-step float bound by [0,5]
inline bool CheckRating(float r) {
  return ((r >= 0) && (r <= 5) &&
      (((r - 0.5) - static_cast<int>(r)) == 0 || (r - static_cast<int>(r)) == 0));
}

}  // unnamed namespace

namespace logging {
namespace event {

class CanonicalObjectsValidator {
 public:
  static bool Validate(const tLogElement& element, const tSearchContext& search_context) {
    if (!Validate(element, search_context.search)) return false;
    // TODO(otasevic): validate FiltersState when it is available
    if (!Validate(element, search_context.sort)) return false;
    // TODO(otasevic): validate SearchExpts when it is available
    return true;
  }

  static bool Validate(const tLogElement& element, const tHotel& hotel) {
    // TODO(otasevic): add constraints for hid
    if (!CheckRating(hotel.stars)) return false;
    if (!CheckRating(hotel.user_rating)) return false;

    return true;
  }

  static bool Validate(const tLogElement& element, const tRate& rate) {
    if (rate.base < 0) return false;
    if (rate.first < 0 && rate.first != -1) return false;
    if (rate.tot < 0) return false;
    return true;
  }

  static bool Validate(const tLogElement& element, const tSort& sort) {
    // TODO(otasevic): move this to constants file
    static const set<string> known_sort_types = {"r/cheap", "r/BestUserRating", "r/BestStarRating",
                                                 "r/Distance"};
    // If the id is not in the group and it is not empty, return invalid
    if (known_sort_types.find(sort.id) == known_sort_types.end() && !(sort.id.empty())) return false;
    return true;
  }

  static bool Validate(const tLogElement& element, const tSearch& search) {
    if (!CheckLatitude(search.lat)) return false;
    if (!CheckLongitude(search.lon)) return false;
    if (!CheckRooms(search.rooms)) return false;
    if (!CheckGuests(search.guests)) return false;
    if (!CanonicalObjectsValidator::Validate(element, search.hotel)) return false;
    if (!CheckNumNights(search.dateless, search.num_nights)) return false;
    if (!CheckDaysUntilCheckin(search.days_until_checkin)) return false;
    if (!CheckStayDates(search.dateless, search.checkin, search.checkout)) return false;
    return true;
  }

  static bool Validate(const tLogElement& element, const tSponsoredRate& sponsored_rate) {
    if (sponsored_rate.rate <= 0) return false;
    return true;
  }

  static bool Validate(const tLogElement& element, const tSponsoredAd& sponsored_ad) {
    if (sponsored_ad.pos < 0) return false;
    return true;
  }

  // tBriefRate is not always available, so check if it is available
  static bool Validate(const tLogElement& element, const tBriefRate& brief_rate) {
    // if values are all default, brief_rate is not available and that is valid
    if (brief_rate.source.empty() && brief_rate.tot == 0 && brief_rate.base == 0 &&
        !brief_rate.conv && brief_rate.curr.empty()) return true;

    // if some values are not default, check the validity of the fields separately
    if (brief_rate.base <= 0) return false;
    // TODO(otasevic): For now we are allowing tot=-1, but we need to revisit this
    if (brief_rate.tot <= 0 && brief_rate.tot != -1) return false;
    if (brief_rate.source.empty()) return false;

    return true;
  }

  static bool Validate(const tLogElement& element, const tBookParams& book_params) {
    if (book_params.base <= 0) return false;
    if (book_params.tot < 0) return false;
    if (!CheckStayDates(false, book_params.check_in, book_params.check_out)) return false;
    if (book_params.guests <= 0) return false;
    return true;
  }

  static bool Validate(const tLogElement& element, const tImpressionContext& imp) {
    if (!Validate(element, imp.brief_rate)) return false;
    if (!Validate(element, imp.hotel)) return false;
    if (imp.position < 0) return false;
    return true;
  }
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_VALIDATOR_CANONICAL_OBJECTS_CANONICAL_OBJECTS_VALIDATOR_H_
