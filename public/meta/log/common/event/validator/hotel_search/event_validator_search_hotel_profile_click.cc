// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_hotel_search.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventSearchHotelProfileClickValidator : public ValidatorForType<tEventSearchHotelProfileClick> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventSearchHotelProfileClick& event) const {
    // TODO(otasevic): enable brief_rate validation after it is resolved
    /*
    if (!(CanonicalObjectsValidator::Validate(element, event.search_context))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.hotel))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.brief_rate))) return 0;
    if (event.current_pos < 0) return 0;
    if (event.view != "list" && event.view!= "map" && element.channel == "web") return 0;*/
    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_search_hotel_profile_click_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Hotel Search", "Hotel Profile Click"),
                        [] { return new EventSearchHotelProfileClickValidator(); });
  });

}  // namespace event
}  // namespace logging
