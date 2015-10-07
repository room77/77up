// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_monetized_click.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventMClickSerpValidator : public ValidatorForType<tEventMClickSerp> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventMClickSerp& event) const {
    if (!(CanonicalObjectsValidator::Validate(element, event.hotel))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.rate))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.search_context))) return 0;
    if (event.position < 0) return 0;
    if (event.num_additional_rates < 0) return 0;
    if (event.range_min < 0) return 0;
    if (event.range_max < 0) return 0;

    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_mclick_serp_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Monetized Click", "Serp"),
                        [] { return new EventMClickSerpValidator(); });
  });

}  // namespace event
}  // namespace logging
