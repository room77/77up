// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_monetized_click.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventMClickExpandedCardValidator : public ValidatorForType<tEventMClickExpandedCard> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventMClickExpandedCard& event) const {
    if (!(CanonicalObjectsValidator::Validate(element, event.rate))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.hotel))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.search_context))) return 0;

    if (event.position < 0) return 0;

    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_mclick_expanded_card_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Monetized Click", "Expanded Card"),
                        [] { return new EventMClickExpandedCardValidator(); });
  });

}  // namespace event
}  // namespace logging
