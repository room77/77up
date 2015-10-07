// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_hotel_search.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventSearchImpressionsValidator : public ValidatorForType<tEventSearchImpression> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventSearchImpression& event) const {
    if (!(CanonicalObjectsValidator::Validate(element, event.search_context))) return 0;
    for (const tImpressionContext& imp : event.impressions) {
      if (!(CanonicalObjectsValidator::Validate(element, imp))) return 0;
    }
    if (event.view != "list" && event.view!= "map" && element.channel == "web") return 0;
    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_search_impressions_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Hotel Search", "Impressions"),
                        [] { return new EventSearchImpressionsValidator(); });
  });

}  // namespace event
}  // namespace logging
