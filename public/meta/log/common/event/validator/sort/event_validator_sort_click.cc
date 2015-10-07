// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_sort.h"
#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class SortClickValidator : public ValidatorForType<tEventSortClick> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventSortClick& event) const {
    if (!(CanonicalObjectsValidator::Validate(element, event.sort))) return 0;
    return event::ValidatorBase::kEventDataBit;  // set event data bit to 1;
  }
};

INIT_ADD_REQUIRED("sort_click_validator", [] {
  // register the validator class to the ValidatorBase factory
  ValidatorBase::bind(pair<string, string> ("Sort", "Click"),
                      [] { return new SortClickValidator(); });
});

}  // namespace event
}  // namespace logging

