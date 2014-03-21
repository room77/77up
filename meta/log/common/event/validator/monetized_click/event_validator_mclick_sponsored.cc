// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_monetized_click.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventMClickSponsoredValidator : public ValidatorForType<tEventMClickSponsored> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventMClickSponsored& event) const {
    if (!(CanonicalObjectsValidator::Validate(element, event.search_context))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.sponsored_ad))) return 0;
    if (event.view != "list" && event.view != "map") return 0;

    // TODO(otasevic): change this
    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_mclick_sponsored_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Monetized Click", "Sponsored"),
                        [] { return new EventMClickSponsoredValidator(); });
  });

}  // namespace event
}  // namespace logging
