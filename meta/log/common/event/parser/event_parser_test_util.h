// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_COMMON_EVENT_PARSER_EVENT_PARSER_TEST_UTIL_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_PARSER_EVENT_PARSER_TEST_UTIL_H_

#include <memory>

#include "base/defs.h"
#include "base/logging.h"
#include "meta/log/common/event/parser/event_parser.h"
#include "test/cc/test_main.h"

namespace logging {
namespace event {
namespace test {

// Test util class for event parsers.
template<typename T>
class EventParserTestUtil : public ::testing::Test {
 public:
  EventParserTestUtil() = default;
  EventParserTestUtil(const string& category, const string& action)
      : category_(category), action_(action) {}

  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    parser_ = EventParserInterface::make_shared(
        EventParserJSONKey(category_, action_));
    ASSERT_NOTNULL(parser_);
  }

  bool ParseStr(const string& str) {
    tLogElement element;
    element.value = str;
    event_ = parser_->Parse(element);
    if (event_ != nullptr) {
      event_data_ = GetEventData<T>(event_.get());
      return true;
    }

    return false;
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  string category_;
  string action_;

  EventParserInterface::shared_proxy parser_;
  shared_ptr<tEventBase> event_;
  T* event_data_ = nullptr;
};

}  // namespace test
}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_PARSER_EVENT_PARSER_TEST_UTIL_H_
