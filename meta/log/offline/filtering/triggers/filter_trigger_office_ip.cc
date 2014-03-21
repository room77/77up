// Copyright 2014 Room77, Inc.
// Author: Nikola Otasevic

#include "meta/log/common/event/trigger/event_trigger.h"

#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "meta/log/offline/filtering/end_point_interface.h"
#include "meta/log/offline/filtering/log_writer_manager.h"
#include "util/init/init.h"

FLAG_string(office_ip, "50.76.63.141",
            "Office ip");

namespace logging {
namespace event {

// this trigger filters out all the log elements coming from the office ip
class OfficeIpFilterTrigger : public EventsTriggerWithoutData {
 protected:
  virtual void HandleTrigger(const tLogElement& element) const {
    // write all the logs that are not coming from the office ip
    if (element.user_ip != gFlag_office_ip) {
      // write if ip is not office ip
      // TODO(otasevic): move this id to INIT_ADD
      EndPointInterface::shared_proxy end_point =
              EndPointInterface::make_shared("UserLogsEndPoint");
      // LogWriter log_writer(end_point->GetEndPoint(element));
      // log_writer.WriteLog(element);
      LogWriterManager::Instance().WriteLogElement(element, end_point->GetEndPoint(element));
    }
  }
};

INIT_ADD_REQUIRED("office_ip_filter_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("OfficeIpFilterTrigger", [] { return new OfficeIpFilterTrigger(); });

  // register trigger for all events in the trigger collection
  EventTriggerCollection::Instance().RegisterTriggerForAllEvents("OfficeIpFilterTrigger", -10);
});

}  // namespace event
}  // namespace logging
