// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_TRIGGER_EVENT_TRIGGER_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_TRIGGER_EVENT_TRIGGER_H_

#include <memory>

#include "base/defs.h"
#include "meta/log/common/event/event.h"
#include "meta/log/common/event/validator/event_validator.h"
#include "meta/log/common/log_datatypes.h"
#include "util/factory/factory.h"

namespace logging {
namespace event {

// interface that enables factory registration for triggers of any type ("all" and "specific")
class TriggerInterface : public Factory<TriggerInterface> {
 public:
  virtual ~TriggerInterface() {}

  virtual void Trigger(const tLogElement& element, shared_ptr<tEventBase> event) const = 0;

  // This method specifies the kind of data a trigger "wants" to receive. By default, triggers will
  // receive only the events for which all parts of data are valid. That means that header and
  // element data have to be valid. Additionally, either parser is null, or if parser is non-null
  // the data has to be valid.

  // Individual triggers may override this method and accept only data that is invalid
  // or any other kind of data.
  virtual bool DataValidationCompatible(const uint8_t log_status) const {
    return ((log_status & ValidatorBase::kHeaderBit) == ValidatorBase::kHeaderBit &&
        (log_status & ValidatorBase::kElementDataBit) == ValidatorBase::kElementDataBit) &&
        (((log_status & ValidatorBase::kParserNotNullBit) == ValidatorBase::kParserNotNullBit &&
            (log_status & ValidatorBase::kEventDataBit) == ValidatorBase::kEventDataBit) ||
            ((log_status & ValidatorBase::kParserNotNullBit) != ValidatorBase::kParserNotNullBit));
  }
};

// Utility class for triggers that do not care about the data associated with an event.
class EventsTriggerWithoutData : public TriggerInterface {
 public:
  virtual ~EventsTriggerWithoutData() {}

  virtual void Trigger(const tLogElement& element, shared_ptr<tEventBase> event) const {
    HandleTrigger(element);
  }

 protected:
  virtual void HandleTrigger(const tLogElement& element) const = 0;
};

// convenience wrapper to make it possible to use handlers with closure
// used by suggest::LogReader
class EventsTriggerWithoutDataWithCallback : public EventsTriggerWithoutData {
  using HandlerType = function<void (const tLogElement& element)>;

 public:
  explicit EventsTriggerWithoutDataWithCallback(HandlerType handler)
      : EventsTriggerWithoutData(), handler_(handler) { }

  virtual ~EventsTriggerWithoutDataWithCallback() { }

 protected:
  virtual void HandleTrigger(const tLogElement& element) const {
    handler_(element);
  }

 private:
  const HandlerType handler_;
};

// Utility class for triggers that require valid data associated with an event.
template<typename T>
class EventTriggerWithData : public TriggerInterface {
  typedef tEventWithData<T> EventT;

 public:
  virtual ~EventTriggerWithData() {}

  virtual void Trigger(const tLogElement& element, shared_ptr<tEventBase> event) const {
    // If the event does nto have valid data, then this trigger cannot be invoked.
    if (event == nullptr) return;
    EventT* event_with_data = dynamic_cast<EventT*>(event.get());
    ASSERT_NOTNULL(event_with_data);
    HandleTrigger(element, event_with_data->data);
  }

 protected:
  virtual void HandleTrigger(const tLogElement& element, const T& event) const = 0;
};

}  // namespace event
}  // namespace logging

#endif  // _PUBLIC_META_LOG_COMMON_EVENT_TRIGGER_EVENT_TRIGGER_H_
