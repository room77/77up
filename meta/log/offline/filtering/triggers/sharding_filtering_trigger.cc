// Copyright 2014 Room77, Inc.
// Author: Nikola Otasevic

#include "meta/log/common/event/trigger/event_trigger.h"

#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "meta/log/offline/filtering/end_point_interface.h"
#include "meta/log/offline/filtering/log_writer_manager.h"
#include "util/init/init.h"

namespace logging {
namespace event {

// this trigger filters out all the log elements coming from the office ip
class MonetizedClickShardingFilteringTrigger : public EventsTriggerWithoutData {
 protected:
  virtual void HandleTrigger(const tLogElement& element) const {
    // TODO(otasevic): move this id to INIT_ADD
    EndPointInterface::shared_proxy end_point =
            EndPointInterface::make_shared("ShardingEndPoint");
    LogWriterManager::Instance().WriteLogElement(element, end_point->GetEndPoint(element));
  }
};

INIT_ADD_REQUIRED("monetized_click_sharding_filtering_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("MonetizedClickShardingFilteringTrigger",
                         [] { return new MonetizedClickShardingFilteringTrigger(); });

  // register trigger for monetized click category events in the trigger collection
  EventTriggerCollection::Instance().RegisterTriggerForCategory("Monetized Click",
      "MonetizedClickShardingFilteringTrigger", -10);
  EventTriggerCollection::Instance().RegisterTriggerForCategory("Hotel Search",
      "MonetizedClickShardingFilteringTrigger", -10);
  EventTriggerCollection::Instance().RegisterTriggerForCategory("Booking",
      "MonetizedClickShardingFilteringTrigger", -10);
  EventTriggerCollection::Instance().RegisterTriggerForCategory("Hotel Profile",
      "MonetizedClickShardingFilteringTrigger", -10);
});

}  // namespace event
}  // namespace logging
