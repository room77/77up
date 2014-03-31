// Copyright 2013 Room77, Inc.
// Author: Nikola Otasevic

#include "meta/log/common/event/trigger/event_trigger.h"

#include "util/init/init.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/log/remote_log.h"
#include "util/network/office.h"


namespace logging {
namespace event {

// this trigger simply writes the individual requests into syslog
class WriteToSysLogTrigger : public EventsTriggerWithoutData {
 protected:
  virtual void HandleTrigger(const tLogElement& element) const {
    // write the element into syslog
    // office ips should go into devweb
    if (::util::Office::IsOfficeIP(element.user_ip)) {
      util::RemoteLog::Instance().Log(serial::Serializer::ToJSON(element),
                                      util::RemoteLog::MsgType::DEVWEB);
    } else {
      util::RemoteLog::Instance().Log(serial::Serializer::ToJSON(element),
                                      util::RemoteLog::MsgType::WEB);
    }
    // TODO(otasevic) dump into binary format
    // util::RemoteLog::Instance().Log(serial::Serializer::ToBinary(element),
                                     // util::RemoteLog::MsgType::[TODO custom message type]);
  }
};

INIT_ADD_REQUIRED("write_to_syslog_trigger", [] {
  // register the trigger class to the TriggerInterface factory
  TriggerInterface::bind("WriteToSysLogTrigger", [] { return new WriteToSysLogTrigger(); });

  // register trigger for all events in the trigger collection
  EventTriggerCollection::Instance().RegisterTriggerForAllEvents("WriteToSysLogTrigger", -10);
});

}  // namespace event
}  // namespace logging
