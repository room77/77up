// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_sort.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/counter/counter.h"

namespace logging {
namespace event {

class SortClickTrigger : public EventTriggerWithData<tEventSortClick> {
 public:
  virtual ~SortClickTrigger() {}

 protected:
  virtual void HandleTrigger(const tLogElement& element,
                             const tEventSortClick& event) const {

    // TODO(otasevic): add a counter for this event;
  }
};

INIT_ADD_REQUIRED("sort_click_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("SortClickTrigger", [] { return new SortClickTrigger(); });

  // register trigger
  EventTriggerCollection::Instance().RegisterTriggerForCategoryAction(
      "Sort", "click", "SortClickTrigger", -10);
});

}  // namespace event
}  // namespace logging

