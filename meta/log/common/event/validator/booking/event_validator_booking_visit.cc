// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_booking.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventBookingVisitValidator : public ValidatorForType<tEventBookingVisit> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventBookingVisit& event) const {
    if(!(CanonicalObjectsValidator::Validate(element, event.book_params))) return 0;

    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_booking_visit_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Booking", "Visit"),
                        [] { return new EventBookingVisitValidator(); });
  });

}  // namespace event
}  // namespace logging
