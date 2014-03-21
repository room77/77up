// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_VALIDATOR_EVENT_VALIDATOR_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_VALIDATOR_EVENT_VALIDATOR_H_

#include <memory>
#include <utility>

#include "base/defs.h"
#include "meta/log/common/event/event.h"
#include "meta/log/common/log_datatypes.h"
#include "util/factory/factory.h"


namespace logging {
namespace event {

// Interface that enables factory registration for validators
class ValidatorBase : public Factory<ValidatorBase, pair<string, string> > {
 public:
  virtual ~ValidatorBase() {}

  // This method is implemented by each derived class in order to specify the invariants for
  // the values of the event fields. For example, a price should always be >0. Number of stars
  // should be between 0-5 etc.
  virtual uint8_t ValidateEventData(const tLogElement& element,
                                    shared_ptr<tEventBase> event) const = 0;

  // Each part of the data has its corresponding bit positions that tells us whether data is
  // valid (1) or invalid (0)
  static constexpr uint8_t kHeaderBit = 1<< 0;
  static constexpr uint8_t kElementDataBit = 1<< 1;
  static constexpr uint8_t kParserNotNullBit = 1<< 2;
  static constexpr uint8_t kEventNotNullBit = 1<< 3;
  static constexpr uint8_t kEventDataBit = 1<< 4;
};

// Utility class for validators of specific event types
template<typename T>
class ValidatorForType : public ValidatorBase {
  typedef tEventWithData<T> EventT;

 public:
  virtual ~ValidatorForType() {}

  virtual uint8_t ValidateEventData(const tLogElement& element,
                                    shared_ptr<tEventBase> event) const {
    if (event == nullptr) return 0;
    EventT* event_with_data = dynamic_cast<EventT*>(event.get());
    ASSERT_NOTNULL(event_with_data);
    return HandleValidation(element, event_with_data->data);
  }

 protected:
  virtual uint8_t HandleValidation(const tLogElement& element, const T& event) const = 0;
};

}  // namespace event
}  // namespace logging

#endif  // _PUBLIC_META_LOG_COMMON_EVENT_VALIDATOR_EVENT_VALIDATOR_H_
