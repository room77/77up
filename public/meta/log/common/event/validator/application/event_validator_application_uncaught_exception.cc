// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/event_validator.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_application.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace event {

class EventApllicationUncaughtExceptionValidator :
    public ValidatorForType<tEventApplicationUncaughtException> {
 protected:
  virtual uint8_t HandleValidation(const tLogElement& element,
                                   const tEventApplicationUncaughtException& event) const {
    // if (event.line_num < 0) return 0;  TODO(otasevic): figure out what the checks should be

    return event::ValidatorBase::kEventDataBit;
  }
};

INIT_ADD_REQUIRED("event_application_uncaught_exception_validator", [] {
    // register the validator class to the ValidatorBase factory
    ValidatorBase::bind(pair<string, string> ("Application", "Uncaught Exception"),
                        [] { return new EventApllicationUncaughtExceptionValidator(); });
  });

}  // namespace event
}  // namespace logging
