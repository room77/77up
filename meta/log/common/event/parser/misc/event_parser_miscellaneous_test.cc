// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

// @include "event_parser_miscellaneous.cc"

#include "meta/log/common/event/datatypes/event_miscellaneous.h"
#include "meta/log/common/event/parser/event_parser.h"
#include "test/cc/test_main.h"


namespace logging {
namespace event {
namespace test {

// for "page exit" event, test whether fields are parsed well (this is the regular case scenario)
TEST(EventParserForTypePageExit, Sanity) {
  EventParserInterface::shared_proxy event_parser =
        EventParserInterface::make_shared(EventParserJSONKey("Page Exit", "Page Exit"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("{\"duration\":911509,\"page\":\"https://titan/web/#/\"}");
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  const tEventPageExit& event_page_exit = GetEventData<tEventPageExit>(*event);
  EXPECT_EQ(911509, event_page_exit.duration);
  EXPECT_EQ("https://titan/web/#/", event_page_exit.page);
}

TEST(EventParserForTypePageExit, ParseEmptyRequest) {
  EventParserInterface::shared_proxy event_parser =
      EventParserInterface::make_shared(EventParserJSONKey("Page Exit", "Page Exit"));
  tLogElement element;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  EXPECT_EQ(nullptr, event);
}

TEST(EventParserForTypePageExit, ParseEmptyString) {
  EventParserInterface::shared_proxy event_parser =
        EventParserInterface::make_shared(EventParserJSONKey("Page Exit", "Page Exit"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("");
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  EXPECT_EQ(nullptr, event);
}

// for "page exit" event, test whether change of type (expected uint_64, get string) causes nullptr
TEST(EventParserForTypePageExit, ParseCorruptedRequest) {
  EventParserInterface::shared_proxy event_parser =
        EventParserInterface::make_shared(EventParserJSONKey("Page Exit", "Page Exit"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("{\"duration\":\"300\"}");
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  EXPECT_EQ(nullptr, event);
}

// for "page exit" event, test whether page and duration fields are parsed well
// even when there is additional information
TEST(EventParserForTypePageExit, ParseRegularWithAdditionalInfoRequest) {
  EventParserInterface::shared_proxy event_parser =
        EventParserInterface::make_shared(EventParserJSONKey("Page Exit", "Page Exit"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("{\"duration\":911509,\"page\":\"https://titan/web/#/\"}");
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  const tEventPageExit& event_page_exit = GetEventData<tEventPageExit>(*event);
  EXPECT_EQ(911509, event_page_exit.duration);
  EXPECT_EQ("https://titan/web/#/", event_page_exit.page);
}

}  // namespace test
}  // namespace event
}  // namespace logging

