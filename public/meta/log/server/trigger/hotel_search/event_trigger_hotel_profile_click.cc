// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_hotel_search.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/counter/counter.h"

namespace logging {
namespace event {

// TODO(otasevic) change this trigger to "with data" trigger once the data struct is agreed on
// and validators are written
class HotelProfileClickTrigger : public EventsTriggerWithoutData {
 public:
  virtual ~HotelProfileClickTrigger() {}

 protected:
  virtual void HandleTrigger(const tLogElement& element) const {
    // Count number of hotel profile clicks
    // Get a mutable shared copy of a "hotel profile click" counter
    counter::CounterBase::mutable_shared_proxy syscounter =
        counter::CounterBase::make_shared("HotelProfileClickCounter");

    counter::metrics::tCountedEvent hotel_profile_click_event;
    syscounter->ProcessEvent(hotel_profile_click_event);
  }
};

INIT_ADD_REQUIRED("hotel_profile_click_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("HotelProfileClickTrigger", [] { return new HotelProfileClickTrigger(); });

  // register trigger
  EventTriggerCollection::Instance().RegisterTriggerForCategoryAction(
      "Hotel Search", "Hotel Profile Click", "HotelProfileClickTrigger", -10);
  // register "Hotel Profile Click" Counter with the factory
  counter::CounterBase::bind("HotelProfileClickCounter", [] { return new counter::Counter(); });

  counter::CounterBase::pin(counter::CounterBase::make_shared("HotelProfileClickCounter"));
});

}  // namespace event
}  // namespace logging

