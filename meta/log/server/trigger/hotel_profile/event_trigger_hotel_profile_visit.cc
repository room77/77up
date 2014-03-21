// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/datatypes/event_hotel_profile.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/counter/counter.h"

namespace logging {
namespace event {

class HotelProfileVisitTrigger : public EventTriggerWithData<tEventHotelProfileVisit> {
 public:
  virtual ~HotelProfileVisitTrigger() {}

 protected:
  virtual void HandleTrigger(const tLogElement& element,
                             const tEventHotelProfileVisit& event) const {
    // Count number of hotel profile visits
    // Get a mutable shared copy of a "hotel profile visit" counter
    counter::CounterBase::mutable_shared_proxy syscounter =
        counter::CounterBase::make_shared("HotelProfileVisitCounter");

    counter::metrics::tCountedEvent hotel_profile_visit_event;
    syscounter->ProcessEvent(hotel_profile_visit_event);
  }
};

INIT_ADD_REQUIRED("hotel_profile_visit_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("HotelProfileVisitTrigger", [] { return new HotelProfileVisitTrigger(); });

  // register trigger
  EventTriggerCollection::Instance().RegisterTriggerForCategoryAction(
      "Hotel Profile", "Visit", "HotelProfileVisitTrigger", -10);
  // register "Hotel Profile Click" Counter with the factory
  counter::CounterBase::bind("HotelProfileVisitCounter", [] { return new counter::Counter(); });

  counter::CounterBase::pin(counter::CounterBase::make_shared("HotelProfileVisitCounter"));
});

}  // namespace event
}  // namespace logging

