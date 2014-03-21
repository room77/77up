// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

// @include "event_parser_hotel_search.cc"

#include "meta/log/common/event/datatypes/event_hotel_search.h"

#include "meta/log/common/event/parser/event_parser.h"
#include "test/cc/test_main.h"

namespace logging {
namespace event {
namespace test {

// for "SearchExps" event, test whether fields are parsed well (this is the regular case scenario)
TEST(EventParserForTypeSearchExps, Sanity) {
  EventParserInterface::shared_proxy event_parser =
        EventParserInterface::make_shared(EventParserJSONKey("Hotel Search", "Search Exps"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("{\"city\":\"IE\",\"ctrseg\":\"IE\",\"dist\":\"A\",\"mic\":\"IA\","
      "\"session\":\"IE\",\"hid\":0}");
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  const tEventSearchExps& event_search_exps = GetEventData<tEventSearchExps>(*event);
  EXPECT_EQ("IE", event_search_exps.city);
  EXPECT_EQ("IE", event_search_exps.ctrseg);
  EXPECT_EQ("A", event_search_exps.dist);
  EXPECT_EQ("IA", event_search_exps.mic);
  EXPECT_EQ("IE", event_search_exps.session);
  EXPECT_EQ(0, event_search_exps.hid);
}

// for "SearchExps" event, test whether small corruptions in value are caught
TEST(EventParserForTypeSearchExps, ParseCorruptedRequest1) {
  EventParserInterface::shared_proxy event_parser =
      EventParserInterface::make_shared(EventParserJSONKey("Hotel Search", "Search Exps"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("{\"city\":[]}");  //  city is supposed to be string, so it's corrupted here
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  EXPECT_EQ(nullptr, event);
}

// TODO(otasevic): Not sure whether we want ints to be forced into string or just fail
// for "SearchExps" event, test whether small corruptions in value are caught
TEST(EventParserForTypeSearchExps, ParseCorruptedRequest2) {
  EventParserInterface::shared_proxy event_parser =
      EventParserInterface::make_shared(EventParserJSONKey("Hotel Search", "Search Exps"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("{\"city\":5000sth}");  //  city is supposed to be string, so it's corrupted here
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);

  const tEventSearchExps& event_search_exps = GetEventData<tEventSearchExps>(*event);
  EXPECT_EQ("5000sth", event_search_exps.city);
}

// for "SearchExps" event, test that parser returns null when value is an empty string
TEST(EventParserForTypeSearchExps, EmptyString) {
  EventParserInterface::shared_proxy event_parser =
        EventParserInterface::make_shared(EventParserJSONKey("Hotel Search", "Search Exps"));
  tLogElement element;
  serial::types::ArbitBlob value;
  value.FromJSON("");
  element.value = value;
  shared_ptr<tEventBase> event = event_parser->Parse(element);
  EXPECT_EQ(nullptr, event);
}

}  // namespace test
}  // namespace event
}  // namespace logging

