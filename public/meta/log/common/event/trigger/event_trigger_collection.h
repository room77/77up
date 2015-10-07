// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_TRIGGER_EVENT_TRIGGER_COLLECTION_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_TRIGGER_EVENT_TRIGGER_COLLECTION_H_

#include <map>
#include <unordered_map>
#include <utility>

#include "base/defs.h"
#include "meta/log/common/event/trigger/event_trigger.h"
#include "util/templates/container_util.h"

namespace logging {
namespace event {

// this singleton class provides methods for trigger registration
// and enables retrieval of the registration information
class EventTriggerCollection {
 public:
  // List of triggers.
  typedef vector<TriggerInterface::shared_proxy> TriggersList;
  // map from priority to vector of trigger ids
  typedef map<int, TriggersList> PriorityTriggersMap;

  static EventTriggerCollection& Instance() {
    static EventTriggerCollection the_one;
    return the_one;
  }

  const PriorityTriggersMap& GetTriggersForAllEvents() const {
    static const PriorityTriggersMap kDefault;
    return ::util::tl::FindWithDefault(trigger_map(), CategoryActionKey("*", "*"), kDefault);
  }

  const PriorityTriggersMap& GetTriggersForCategory(const string& category) const {
    static const PriorityTriggersMap kDefault;
    return ::util::tl::FindWithDefault(trigger_map(), CategoryActionKey(category, "*"), kDefault);
  }

  const PriorityTriggersMap& GetTriggersForCategoryAction(const string& category,
                                                          const string& action) const {
    static const PriorityTriggersMap kDefault;
    return ::util::tl::FindWithDefault(trigger_map(), CategoryActionKey(category, action),
                                       kDefault);
  }

  // Registers a trigger for a specific category, action pair at the given priority.
  bool RegisterTriggerForCategoryAction(const string& category, const string& action,
                                        const string& trigger_id,
                                        int trigger_priority = 0);

  bool RegisterTriggerForCategoryMultipleActions(const string& category,
                                                 const vector<string>& actions,
                                                 const string& trigger_id,
                                                 int trigger_priority = 0) {
    bool res = true;
    for (const string& action : actions) {
      if (RegisterTriggerForCategoryAction(category, action, trigger_id, trigger_priority))
        continue;

      // We failed.
      res = false;
      break;
    }
    return res;
  }

  // Registers a trigger for a given category pair at the given priority.
  // This trigger will be invoked for all actions associated with the category.
  // Here are some priority guidelines:
  // 1. Priorities are executed in the increasing order (-100 before 100)
  // 2. Default priority is 0 (most offline triggers)
  // 3. Most online triggers have a priority -10
  // 4. Use [-20, 20] only if your triggers need to be executed before or after other general
  // triggers (for example, if you are writing a trigger that modifies
  // the data to be used by other triggers).
  // 5. Don't use a priority outside of [-20, 20] range
  bool RegisterTriggerForCategory(const string& category,
                                  const string& trigger_id, int trigger_priority = 0) {
    return RegisterTriggerForCategoryAction(category, "*", trigger_id, trigger_priority);
  }


  // Registers a trigger for all events.
  bool RegisterTriggerForAllEvents(const string& trigger_id,
                                   int trigger_priority = 0) {
    return RegisterTriggerForCategoryAction("*", "*", trigger_id, trigger_priority);
  }

 protected:
  EventTriggerCollection() = default;

  // represents (category, action) pair
  typedef pair<string, string> CategoryActionKey;

  // map from (category, action) pair to a triggers priority map
  typedef unordered_map<CategoryActionKey, PriorityTriggersMap> TriggersMap;

  const TriggersMap& trigger_map() const {
    return triggers_map_;
  }

 private:
  // mapping from pair of strings (category, action) to a map that maps
  // priority level to a set of triggers associated with the priority level
  TriggersMap triggers_map_;
};

// used by suggest::LogReader
inline void AddTriggerHandlerWithoutData(
    const string& category, const string& action, const string& trigger_id,
    function<void (const tLogElement&)> handler) {

  TriggerInterface::bind(trigger_id, [&] {
    return new EventsTriggerWithoutDataWithCallback(handler); });

  EventTriggerCollection::Instance().RegisterTriggerForCategoryAction(
      category, action, trigger_id, -10);
}

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_TRIGGER_EVENT_TRIGGER_COLLECTION_H_
