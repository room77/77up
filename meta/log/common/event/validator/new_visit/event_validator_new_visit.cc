// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_miscellaneous.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class NewVisitValidator : public ValidatorForType<tEventNewVisit> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element, const tEventNewVisit& event) const {
    // check if the value string is either empty or a proper url
    return event::ValidatorBase::kEventDataBit;  // set event data bit to 1;
  }
};

INIT_ADD_REQUIRED("new_visit_validator", [] {
  // register the validator class to the ValidatorBase factory
  ValidatorBase::bind(pair<string, string> ("New Visit", "Referrer"),
                      [] { return new NewVisitValidator(); });
});

}  // namespace event
}  // namespace logging

