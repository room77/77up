// Copyright 2013 Room77, Inc.
// Author: Nikola Otasevic

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/counter/counter.h"
#include "util/log/remote_log.h"
#include "meta/log/common/event/constants.h"

namespace logging {
namespace event {

// This trigger gets all the data that is invalid and writes it to a special syslog
class WriteToBadSysLogTrigger : public EventsTriggerWithoutData {
 public:
  // Overriding the method to return true when the data is invalid
  virtual bool DataValidationCompatible(const uint8_t log_status) const {
    return !(((log_status & ValidatorBase::kHeaderBit) == ValidatorBase::kHeaderBit &&
        (log_status & ValidatorBase::kElementDataBit) == ValidatorBase::kElementDataBit) &&
        (((log_status & ValidatorBase::kParserNotNullBit) == ValidatorBase::kParserNotNullBit &&
            (log_status & ValidatorBase::kEventDataBit) == ValidatorBase::kEventDataBit) ||
            ((log_status & ValidatorBase::kParserNotNullBit) != ValidatorBase::kParserNotNullBit)));
  }

 protected:
  virtual void HandleTrigger(const tLogElement& element) const {
    // write the element into bad syslog
    if (critical_events.find(
        pair<string, string> (element.category, element.action)) != critical_events.end()) {
      counter::CounterBase::mutable_shared_proxy syscounter =
          counter::CounterBase::make_shared("InvalidLogsCriticalEventsCounter");

      counter::metrics::tCountedEvent critical_bad_event;
      syscounter->ProcessEvent(critical_bad_event);
    }
    util::RemoteLog::Instance().Log(serial::Serializer::ToJSON(element),
                                     util::RemoteLog::MsgType::BADWEB);
  }
 };

INIT_ADD_REQUIRED("write_to_bad_syslog_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("WriteToBadSysLogTrigger", [] { return new WriteToBadSysLogTrigger(); });

  // register trigger for all events in the trigger collection
  EventTriggerCollection::Instance().RegisterTriggerForAllEvents("WriteToBadSysLogTrigger", -10);

  // register counters for critical events
  counter::CounterBase::bind("InvalidLogsCriticalEventsCounter",
                             [] { return new counter::Counter(); });

  counter::CounterBase::pin(counter::CounterBase::make_shared("InvalidLogsCriticalEventsCounter"));
});

}  // namespace event
}  // namespace logging
