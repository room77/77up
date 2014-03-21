// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_monetized_click.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/counter/counter.h"

namespace logging {
namespace event {

// TODO(otasevic) write a comment for what this trigger does
class MonetizedClickTrigger : public EventsTriggerWithoutData {
 public:
  virtual ~MonetizedClickTrigger() {}

 protected:
  virtual void HandleTrigger(const tLogElement& element) const {
    // Count number of monetized clicks
    // Get a mutable shared copy of a "monetized click" counter
    counter::CounterBase::mutable_shared_proxy syscounter =
        counter::CounterBase::make_shared("MonetizedClickCounter");

    counter::metrics::tCountedEvent monetized_click_event;
    syscounter->ProcessEvent(monetized_click_event);
  }
};

INIT_ADD_REQUIRED("monetized_click_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("MonetizedClickTrigger", [] { return new MonetizedClickTrigger(); });

  // register trigger for all the actions in the monetized click category
  EventTriggerCollection::Instance().RegisterTriggerForCategory("Monetized Click",
                                                                "MonetizedClickTrigger", -10);
  // register "Monetized Click" Counter with the factory
  counter::CounterBase::bind("MonetizedClickCounter", [] { return new counter::Counter(); });

  counter::CounterBase::pin(counter::CounterBase::make_shared("MonetizedClickCounter"));
});

}  // namespace event
}  // namespace logging

