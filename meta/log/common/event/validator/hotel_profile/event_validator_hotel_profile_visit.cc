// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_hotel_profile.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class HotelProfileVisitValidator : public ValidatorForType<tEventHotelProfileVisit> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventHotelProfileVisit& event) const {
    if (!(CanonicalObjectsValidator::Validate(element, event.hotel))) return 0;
    return event::ValidatorBase::kEventDataBit;  // set event data bit to 1;
  }
};

INIT_ADD_REQUIRED("hotel_profile_visit_validator", [] {
  // register the validator class to the ValidatorBase factory
  ValidatorBase::bind(pair<string, string> ("Hotel Profile", "Visit"),
                      [] { return new HotelProfileVisitValidator(); });
});

}  // namespace event
}  // namespace logging

