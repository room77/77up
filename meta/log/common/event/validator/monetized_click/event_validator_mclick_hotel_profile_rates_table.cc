// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_monetized_click.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventMClickHotelProfileRatesTableValidator : public ValidatorForType<tEventMClickHotelProfileRatesTable> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventMClickHotelProfileRatesTable& event) const {
    // TODO(otasevic): re-enable this validation
/*    if (!(CanonicalObjectsValidator::Validate(element, event.hotel))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.rate))) return 0;
    if (!(CanonicalObjectsValidator::Validate(element, event.search_context))) return 0;
    if (event.position_within_source < 0) return 0;
    if (event.source_position < 0) return 0;
    if (event.num_sources <= 0) return 0;
    if (event.num_rates_from_source <= 0 && element.channel == "web") return 0;*/

    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_mclick_hotel_profile_rates_table_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Monetized Click", "Hotel Profile Rates Table"),
                        [] { return new EventMClickHotelProfileRatesTableValidator(); });
  });

}  // namespace event
}  // namespace logging
