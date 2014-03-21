// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_PARSER_EVENT_PARSER_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_PARSER_EVENT_PARSER_H_

#include <memory>
#include <tuple>
#include <utility>

#include "base/logging.h"
#include "meta/log/common/event/event.h"
#include "meta/log/common/log_datatypes.h"
#include "util/factory/factory.h"
#include "util/serial/serializer.h"


namespace logging {
namespace event {

// The key for event parsers.
// Tuple of <serialization_type,  Category, Action>
// e.g. <"json", "New Visit", "Referrer">
class EventParserKey : public tuple<string, string, string> {
 public:
  using tuple<string, string, string>::tuple;

  static const string& JSONKey() {
    static const string kKey = "json";
    return kKey;
  }

  static const string& BinaryKey() {
    static const string kKey = "binary";
    return kKey;
  }
};

class EventParserJSONKey : public EventParserKey {
 public:
  using EventParserKey::EventParserKey;

  EventParserJSONKey(const string& category, const string& action)
      : EventParserKey(JSONKey(), category, action) {}
};

class EventParserBinaryKey : public EventParserKey {
 public:
  using EventParserKey::EventParserKey;

  EventParserBinaryKey(const string& category, const string& action)
      : EventParserKey(BinaryKey(), category, action) {}
};

// interface that enables factory registration for parsers
class EventParserInterface : public Factory<EventParserInterface, EventParserKey> {
 public:
  virtual ~EventParserInterface() {}

  virtual shared_ptr<tEventBase> Parse(const tLogElement& element) const = 0;
};

// Utility class for parsers that parse specific events where the type of value is specified
// through template parameter T.
template<typename T>
class EventParserForType : public EventParserInterface {
  typedef tEventWithData<T> EventT;

 public:
  virtual ~EventParserForType() {}

  virtual shared_ptr<tEventBase> Parse(const tLogElement& element) const {
    EventT* event = new EventT;
    shared_ptr<tEventBase> shared_event(event);

    // this check will fail if the value is corrupted, i.e. either the stream is corrupted
    // or we have a type mismatch.
    // in that case we want to set the pointer to null
    if (!ParseFromString(element.value, &(event->data))) {
      VLOG(3) << "Failed to parse ---" << element.value << "--- for ["
              << element.category << ", " << element.action << "]";
      shared_event.reset();
    }
    return shared_event;
  }

 protected:
  // Parses the object from the corresponding string.
  virtual bool ParseFromString(const string& str, T* event_data) const = 0;
};

// Utility class for parsers that parse an event from JSON stream.
template<typename T>
class EventJSONParserForType : public EventParserForType<T> {
 public:
  virtual ~EventJSONParserForType() {}

 protected:
  // Parses the object from the corresponding string.
  virtual bool ParseFromString(const string& str, T* event_data) const {
    return serial::Serializer::FromJSON(str, event_data);
  }
};

// Utility class for parsers that parse an event from BINARY stream.
template<typename T>
class EventBinaryParserForType : public EventParserForType<T> {
 public:
  virtual ~EventBinaryParserForType() {}

 protected:
  // Parses the object from the corresponding string.
  virtual bool ParseFromString(const string& str, T* event_data) const {
    return serial::Serializer::FromBinary(str, event_data);
  }
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_PARSER_EVENT_PARSER_H_
