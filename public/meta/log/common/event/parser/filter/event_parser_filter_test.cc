// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// @include "event_parser_filter.cc"

#include "meta/log/common/event/datatypes/event_filter.h"

#include "meta/log/common/event/parser/event_parser.h"
#include "meta/log/common/event/parser/event_parser_test_util.h"
#include "test/cc/test_main.h"

namespace logging {
namespace event {
namespace test {

class EventJSONParserForFilterSelectedItemTest
    : public EventParserTestUtil<tEventFilterSelectedItem> {
 public:
  EventJSONParserForFilterSelectedItemTest() : EventParserTestUtil("Filter", "Star Rating") {}
};

TEST_F(EventJSONParserForFilterSelectedItemTest, Sanity) {
  {
    EXPECT_FALSE(ParseStr(""));
  }
  {
    EXPECT_TRUE(ParseStr("2-Star: false"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("2-Star", event_data_->name);
    EXPECT_FALSE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("2-Star: true"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("2-Star", event_data_->name);
    EXPECT_TRUE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("ADA:Accessible: true"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("ADA:Accessible", event_data_->name);
    EXPECT_TRUE(event_data_->state);
  }
}

TEST_F(EventJSONParserForFilterSelectedItemTest, EscapedString) {
  {
    EXPECT_FALSE(ParseStr("\"\""));
  }
  {
    EXPECT_TRUE(ParseStr("\"2-Star: false\""));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("2-Star", event_data_->name);
    EXPECT_FALSE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("\"2-Star: true\""));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("2-Star", event_data_->name);
    EXPECT_TRUE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("\"ADA:Accessible: true\""));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("ADA:Accessible", event_data_->name);
    EXPECT_TRUE(event_data_->state);
  }
}

class EventJSONParserForFilterNeighborhoodTest
    : public EventParserTestUtil<tEventFilterSelectedItem> {
 public:
  EventJSONParserForFilterNeighborhoodTest() : EventParserTestUtil("Filter", "Neighborhood") {}
};

TEST_F(EventJSONParserForFilterNeighborhoodTest, Sanity) {
  {
    EXPECT_FALSE(ParseStr(""));
  }
  {
    EXPECT_TRUE(ParseStr("Hollywood (43): false"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("Hollywood", event_data_->name);
    EXPECT_FALSE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("Hollywood (43): true"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("Hollywood", event_data_->name);
    EXPECT_TRUE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("\"Hollywood (43): false\""));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("Hollywood", event_data_->name);
    EXPECT_FALSE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("\"Hollywood (43): true\""));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ("Hollywood", event_data_->name);
    EXPECT_TRUE(event_data_->state);
  }
}

class EventJSONParserForFilterSpecialRatesTest
    : public EventParserTestUtil<tEventFilterSpecialRates> {
 public:
  EventJSONParserForFilterSpecialRatesTest() : EventParserTestUtil("Filter", "Special Rates") {}
};

TEST_F(EventJSONParserForFilterSpecialRatesTest, Sanity) {
  {
    EXPECT_FALSE(ParseStr(""));
  }
  {
    EXPECT_TRUE(ParseStr("0_1_1_0"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_FALSE(event_data_->aaa);
    EXPECT_TRUE(event_data_->senior);
    EXPECT_TRUE(event_data_->govt);
    EXPECT_FALSE(event_data_->mil);
  }
  {
    EXPECT_TRUE(ParseStr("\"0_1_1_0\""));
    ASSERT_NOTNULL(event_data_);
    EXPECT_FALSE(event_data_->aaa);
    EXPECT_TRUE(event_data_->senior);
    EXPECT_TRUE(event_data_->govt);
    EXPECT_FALSE(event_data_->mil);
  }
  {
    EXPECT_TRUE(ParseStr(
        R"({"filter_name":"special rates","aaa":0,"senior":1,"govt":1,"mil":0})"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_FALSE(event_data_->aaa);
    EXPECT_TRUE(event_data_->senior);
    EXPECT_TRUE(event_data_->govt);
    EXPECT_FALSE(event_data_->mil);
  }
}

class EventJSONParserForFilterPriceTest
    : public EventParserTestUtil<tEventFilterPrice> {
 public:
  EventJSONParserForFilterPriceTest() : EventParserTestUtil("Filter", "Price") {}
};

TEST_F(EventJSONParserForFilterPriceTest, Sanity) {
  {
    EXPECT_TRUE(ParseStr(""));
  }
  {
    EXPECT_TRUE(ParseStr("140: false"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ(-1, event_data_->min);
    EXPECT_EQ(140, event_data_->max);
    EXPECT_FALSE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr("140: true"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ(-1, event_data_->min);
    EXPECT_EQ(140, event_data_->max);
    EXPECT_TRUE(event_data_->state);
  }
  {
    EXPECT_TRUE(ParseStr(R"("{\"min\":0, \"max\":300}")"));
    ASSERT_NOTNULL(event_data_);
    EXPECT_EQ(0, event_data_->min);
    EXPECT_EQ(300, event_data_->max);
    EXPECT_FALSE(event_data_->state);
  }
}

}  // namespace test
}  // namespace event
}  // namespace logging
