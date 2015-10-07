// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_MANAGER_LOG_MANAGER_H_
#define _PUBLIC_META_LOG_COMMON_MANAGER_LOG_MANAGER_H_

#include <memory>

#include "base/defs.h"
#include "meta/log/common/log_datatypes.h"
#include "meta/log/common/event/event.h"
#include "meta/log/common/event/parser/event_parser.h"
#include "meta/log/common/event/validator/event_validator.h"
#include "util/serial/types/arbit_blob.h"
#include "util/thread/thread_pool.h"

namespace logging {

// this class is a singleton through which all the requests
// are processed. It provides a method that takes in a request and
// manages further processing.
class LogManager {
 public:
  virtual ~LogManager() {}

  static LogManager& Instance() {
    static LogManager the_one;
    return the_one;
  }

  // this method populates the request with additional information coming from
  // the log server and handles calls to parser and trigger invoker
  void ProcessLogs(const vector<serial::types::ArbitBlob>& logs,
                   const tLogRequestInterface& log_request,
                   uint64_t sent_time,
                   uint8_t log_status);

  // Utility method to call ProcessLogs asynchronously.
  void ProcessLogsAsync(const vector<serial::types::ArbitBlob>& logs,
                        shared_ptr<tLogRequestInterface> log_request,
                        uint64_t sent_time);

  // Processes a single log element.
  // The event is only parsed if parse_event is set to true.
  void ProcessLogElement(const shared_ptr<tLogElement>& element,
                         const string& parser_type = event::EventParserKey::JSONKey(),
                         uint8_t log_status = event::ValidatorBase::kHeaderBit |
                         event::ValidatorBase::kElementDataBit);

  // Utility method to call ProcessLogElement asynchronously.
  void ProcessLogElementAsync(const shared_ptr<tLogElement>& element,
                              const string& parser_type = event::EventParserKey::JSONKey());

  // Wait for all logs to have been processed.
  void Wait() const;

  uint8_t ValidateHeaderData(shared_ptr<tLogRequestInterface> log_request) const;

  uint8_t ValidateElementData(shared_ptr<tLogElement> element) const;

 protected:
  LogManager() = default;

  // Executes all the triggers associated with an element
  // (i.e. all events, category, category/action).
  void ExecuteTriggers(const shared_ptr<tLogElement>& element,
                       const shared_ptr<event::tEventBase>& event,
                       uint8_t log_status) const;

  // Thread pool for doing parallel log processing
  static ::util::threading::ThreadPoolFactory::mutable_shared_proxy& pool();
};

}  // namespace logging



#endif  // _PUBLIC_META_LOG_COMMON_MANAGER_LOG_MANAGER_H_
