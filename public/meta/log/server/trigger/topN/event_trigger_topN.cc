// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/counter/counter.h"

namespace logging {
namespace event {

class TopNTrigger : public EventsTriggerWithoutData {
 public:
  virtual ~TopNTrigger() {}

 protected:
  virtual void HandleTrigger(const tLogElement& element) const {

    // Count the number of different topN events
    counter::CounterBase::mutable_shared_proxy syscounter =
        counter::CounterBase::make_shared("TopN " + element.action + " Counter");

    counter::metrics::tCountedEvent topNEvent;
    syscounter->ProcessEvent(topNEvent);
  }
};

INIT_ADD_REQUIRED("topN_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("TopNTrigger", [] { return new TopNTrigger(); });

  // register trigger
  EventTriggerCollection::Instance().RegisterTriggerForCategory(
      "Top N", "TopNTrigger", -10);

  // register "Top N" Counters with the factory
  counter::CounterBase::bind("TopN Dropdown Opened Counter", [] { return new counter::Counter(); });
  counter::CounterBase::pin(counter::CounterBase::make_shared("TopN Dropdown Opened Counter"));

  counter::CounterBase::bind("TopN Category Selected Counter", [] { return new counter::Counter(); });
  counter::CounterBase::pin(counter::CounterBase::make_shared("TopN Category Selected Counter"));
});

}  // namespace event
}  // namespace logging

