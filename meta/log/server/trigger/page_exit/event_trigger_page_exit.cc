// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_miscellaneous.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"


namespace logging {
namespace event {

// TODO(otasevic) write a comment for what this trigger does
class PageExitTrigger : public EventTriggerWithData<tEventPageExit> {
 public:
  virtual ~PageExitTrigger() {}

 protected:
  virtual void HandleTrigger(const tLogElement& element,
                             const tEventPageExit& event) const {
    // TODO(otasevic) Implement the trigger here
  }
};

INIT_ADD_REQUIRED("page_exit_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("PageExitTrigger", [] { return new PageExitTrigger(); });

  // register trigger for a particular (category, action) pair in the trigger collection
  EventTriggerCollection::Instance().RegisterTriggerForCategoryAction("Page Exit", "Page Exit",
      "PageExitTrigger", -10);
});

}  // namespace event
}  // namespace logging

