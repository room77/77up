// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/manager/log_manager.h"

#include <limits>
#include <utility>

#include "base/defs.h"
#include "meta/log/common/event/trigger/event_trigger.h"
#include "meta/log/common/event/trigger/event_trigger_collection.h"
#include "util/serial/serializer.h"

FLAG_int(log_processing_thread_pool_size, 512, "number of threads for log processing");

namespace logging {

using PriorityTriggersMap = event::EventTriggerCollection::PriorityTriggersMap;
using TriggersList = event::EventTriggerCollection::TriggersList;

void LogManager::ExecuteTriggers(const shared_ptr<tLogElement>& element,
                                 const shared_ptr<event::tEventBase>& event,
                                 uint8_t log_status) const {
  const event::EventTriggerCollection& collection = event::EventTriggerCollection::Instance();

  // Get the triggers for all, category, (category,action).
  const PriorityTriggersMap& all_triggers = collection.GetTriggersForAllEvents();
  const PriorityTriggersMap& category_triggers =
      collection.GetTriggersForCategory(element->category);
  const PriorityTriggersMap& category_action_triggers =
      collection.GetTriggersForCategoryAction(element->category, element->action);

  vector<pair<PriorityTriggersMap::const_iterator, PriorityTriggersMap::const_iterator>> iters = {
        {all_triggers.begin(), all_triggers.end()},
        {category_triggers.begin(), category_triggers.end()},
        {category_action_triggers.begin(), category_action_triggers.end()},
      };

  // Iterate through all the iterators, at each iteration find the next iterator with the lowest
  // priority, (break ties in the order opposite to insertion  = CategoryAction -> Category -> All.
  // Execute the triggers associated with that iterator, increment it and move on.
  // If an iterator reaches the end, remove it from the list of iterators, when all triggers have
  // been executed, all iterators will be removed and the loop will end.
  while (iters.size()) {
    // Pair of <priority, index>.
    pair<int, int> best_priority_index(numeric_limits<int>::max(), -1);
    for (int i = iters.size() - 1; i >= 0; --i) {
      const auto& p = iters[i];
      // Check if this iterator has reached its end.
      // If so, remove and continue.
      if (p.first == p.second) {
        iters.erase(iters.begin() + i);

        // The index of the best priority guy will be shifted by 1.
        --best_priority_index.second;
        continue;
      }

      // Check if this iterator has higher priority (= lower numeric value).
      if (p.first->first < best_priority_index.first) {
        best_priority_index = make_pair(p.first->first, i);
      }
    }

    if (best_priority_index.second < 0) {
      // We could not find any index. All iterators reached their end and were removed from the
      // list.
      ASSERT(iters.empty()) << "Still has iterators: " << iters.size();
      break;
    }

    // Get the iterator with the highest priority.
    auto& p = iters[best_priority_index.second];

    // Execute all the triggers.
    // TODO(otasevic, pramodg): Execute them through a priority thread pool.
    // TODO(pramodg): Write priority thread pool class.
    VLOG(4) << "Executing triggers at priority " << best_priority_index.first << " for ("
            << element->category << ", "  << element->action << "). Iter: "
            << best_priority_index.second;
    for (const event::TriggerInterface::shared_proxy& trigger : p.first->second) {
      if (trigger->DataValidationCompatible(log_status)) trigger->Trigger(*element, event);
    }

    // Increment the iterator.
    ++p.first;
  }
}

void LogManager::ProcessLogsAsync(const vector<serial::types::ArbitBlob>& logs,
                                  shared_ptr<tLogRequestInterface> log_request,
                                  uint64_t sent_time) {
  // Check the validity of header info
  uint8_t log_status = ValidateHeaderData(log_request);

  // add the call to the thread pool
  pool()->Add([=]() { ProcessLogs(logs, *log_request, sent_time, log_status); });
}

void LogManager::ProcessLogs(const vector<serial::types::ArbitBlob>& logs,
                             const tLogRequestInterface& log_request,
                             uint64_t sent_time,
                             uint8_t log_status) {
  for (const serial::types::ArbitBlob& log : logs) {
    shared_ptr<tLogElement> element(new tLogElement);
    serial::Serializer::FromJSON(log, element.get());
    element->MergeFrom(log_request);

    // server_time is not a good name, but for now we are double-logging the variable because of
    // consistency with the old logs...corrected_user_time is what should be used from now on
    // estimate of the server time at the time when the event on the client happened
    element->corrected_user_time = element->server_time =
        log_request.created - sent_time + element->user_time;

    log_status |= ValidateElementData(element);

    ProcessLogElement(element, event::EventParserKey::JSONKey(), log_status);
  }
}

// Processes a single log element.
void LogManager::ProcessLogElement(const shared_ptr<tLogElement>& element,
                                   const string& parser_type,
                                   uint8_t log_status) {
  event::EventParserInterface::shared_proxy event_parser =
      event::EventParserInterface::make_shared(
          event::EventParserKey(parser_type, element->category, element->action));

  if (event_parser == nullptr) {
    // See if there is a parser registered with the category.
    event_parser = event::EventParserInterface::make_shared(
        event::EventParserJSONKey(parser_type, element->category, "*"));
  }

  // parser will fail to return a shared pointer in case that no one
  // is registered for a particular (cat, act) pair...so we need the check
  shared_ptr<event::tEventBase> event;
  if (event_parser != nullptr) {
    event = event_parser->Parse(*element);
    // Set a bit that indicates that the parser in present(non-null)
    log_status |= event::ValidatorBase::kParserNotNullBit;
  }
  VLOG(4) << "For keypair:"
          << serial::Serializer::ToJSON(make_pair(element->category, element->action))
          << ": Event Parsed ? "<< (event != nullptr);

  // The event bit should already be 0, this line is just making sure
  log_status &= ~event::ValidatorBase::kEventDataBit;  // set event data bit to 0 (invalid)
  if (event != nullptr) {
    log_status |= event::ValidatorBase::kEventNotNullBit;
    event::ValidatorBase::shared_proxy event_validator =
        event::ValidatorBase::make_shared(make_pair(element->category, element->action));

    // if the validator for an event is not written, the log_status will show valid for the data
    if (event_validator != nullptr) log_status |= event_validator->ValidateEventData(*element,
                                                                                     event);
    else log_status |= event::ValidatorBase::kEventDataBit;  // set event data bit to 1
  }
  // print bit by bit
/*  if(element->category == "New Visit") {
    for (int k = 0; k < 8; ++k) {
      LOG(INFO) << k << "-th bit: " << ((log_status & ( 1 << k )) >> k);
    }
  }*/

  ExecuteTriggers(element, event, log_status);
}

void LogManager::ProcessLogElementAsync(const shared_ptr<tLogElement>& element,
                                        const string& parser_type) {
  pool()->Add([=]() { ProcessLogElement(element, parser_type); });
}

void LogManager::Wait() const {
  pool()->Wait();
}

uint8_t LogManager::ValidateHeaderData(shared_ptr<tLogRequestInterface> log_request) const {
  // TODO(otasevic): add more checks to this function
  if (log_request->created < 0) return 0;

  return event::ValidatorBase::kHeaderBit;  // set header bit to 1
}

uint8_t LogManager::ValidateElementData(shared_ptr<tLogElement> element) const {
  // TODO(otasevic): add more checks to this function

  constexpr int id_size = 6;  // specifies the expected length of the id

  // Check if the id string is alphanumeric and either empty or id_size long
  auto IdValid = [] (const string& id) {
    return ((find_if_not(id.begin(), id.end(), [](const char& c) {return isalnum(c);}) == id.end()) &&
        (id.empty() || id.size() == id_size));
  };
  // Check if id, pid and nid are all alphanumeric, empty or id_size-character strings
  // TODO(otasevic): re-enable 3 lines below once all the ids are in this format
/*  if (!IdValid(element->id)) return 0;
  if (!IdValid(element->pid)) return 0;
  if (!IdValid(element->nid)) return 0;*/

  // If all the checks passed, set the element bit to 1
  return event::ValidatorBase::kElementDataBit;
}

  // Thread pool for doing parallel log processing
::util::threading::ThreadPoolFactory::mutable_shared_proxy& LogManager::pool() {
  // Initialize thread pool.
  static ::util::threading::ThreadPoolFactory::mutable_shared_proxy pool =
      ::util::threading::ThreadPoolFactory::Pool("log_processors_pool",
                                                 gFlag_log_processing_thread_pool_size);
  return pool;
}

}  // namespace logging

