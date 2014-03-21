// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_miscellaneous.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/counter/counter.h"


namespace logging {
namespace event {

// TODO(otasevic) write a comment for what this trigger does
class NewVisitTrigger : public EventTriggerWithData<tEventNewVisit> {
 public:
  virtual ~NewVisitTrigger() {}

 protected:
  virtual void HandleTrigger(const tLogElement& element,
                             const tEventNewVisit& event) const {
    // Count number of new visits
    // Get a mutable shared copy of a "new visit" counter
    counter::CounterBase::mutable_shared_proxy syscounter =
        counter::CounterBase::make_shared("NewVisitCounter");

    counter::metrics::tCountedEvent new_visit_event;
    syscounter->ProcessEvent(new_visit_event, 1);
  }
};

INIT_ADD_REQUIRED("new_visit_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("NewVisitTrigger", [] { return new NewVisitTrigger(); });

  // register trigger for a particular (category, action) pair in the trigger collection
  EventTriggerCollection::Instance().RegisterTriggerForCategoryAction("New Visit", "Referrer",
      "NewVisitTrigger", -10);

  // register "New Visit" Counter with the factory
  counter::CounterBase::bind("NewVisitCounter", [] { return new counter::Counter(); });

  counter::CounterBase::pin(counter::CounterBase::make_shared("NewVisitCounter"));
});

}  // namespace event
}  // namespace logging

