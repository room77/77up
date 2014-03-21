// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/trigger/event_trigger_collection.h"

#include <mutex>

#include "base/defs.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

bool EventTriggerCollection::RegisterTriggerForCategoryAction(const string& category,
    const string& action, const string& trigger_id, int trigger_priority) {
  CategoryActionKey key(category, action);
  LOG(INFO) << "Registering trigger: "<< trigger_id << " for: "
            << serial::Serializer::ToJSON(key) << ", priority: " << trigger_priority;

  event::TriggerInterface::shared_proxy event_trigger =
      event::TriggerInterface::make_shared(trigger_id);

  ASSERT(event_trigger != nullptr) << "Could not find trigger for id: " << trigger_id;

  static mutex triggers_map_mutex;
  lock_guard<mutex> l(triggers_map_mutex);
  triggers_map_[key][trigger_priority].push_back(event_trigger);

  return true;
}

}  // namespace event
}  // namespace logging

