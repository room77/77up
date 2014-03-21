// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/datatypes/event_hotel_search.h"
#include "meta/log/common/event/parser/event_parser.h"

namespace logging {
namespace event {

// registration of parsers associated with the "Hotel Search" category
auto reg_dated_search_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Dated Search"),
    [] { return new EventJSONParserForType<tEventSearch>(); });
auto reg_dated_search_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Dated Search"),
    [] { return new EventBinaryParserForType<tEventSearch>(); });

auto reg_search_load_time_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Load time"),
    [] { return new EventJSONParserForType<tEventSearchLoadTime>(); });
auto reg_search_load_time_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Load time"),
    [] { return new EventBinaryParserForType<tEventSearchLoadTime>(); });

auto reg_search_impressions_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Impressions"),
    [] { return new EventJSONParserForType<tEventSearchImpression>(); });
auto reg_search_impressions_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Impressions"),
    [] { return new EventBinaryParserForType<tEventSearchImpression>(); });

auto reg_search_dateless_search_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Dateless Search"),
    [] { return new EventJSONParserForType<tEventSearch>(); });
auto reg_search_dateless_search_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Dateless Search"),
    [] { return new EventBinaryParserForType<tEventSearch>(); });

auto reg_search_hotel_profile_click_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Hotel Profile Click"),
    [] { return new EventJSONParserForType<tEventSearchHotelProfileClick>(); });
auto reg_search_hotel_profile_click_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Hotel Profile Click"),
    [] { return new EventBinaryParserForType<tEventSearchHotelProfileClick>(); });

auto reg_search_home_page_visit_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Home Page Visit"),
    [] { return new EventJSONParserForType<tEventSearchHomePageVisit>(); });
auto reg_search_home_page_visit_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Home Page Visit"),
    [] { return new EventBinaryParserForType<tEventSearchHomePageVisit>(); });

// TODO(otasevic): deprecate the events below
auto reg_search_detailed_click_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Detailed Click"),
    [] { return new EventJSONParserForType<tEventDetailedClick>(); });
auto reg_search_detailed_click_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Detailed Click"),
    [] { return new EventBinaryParserForType<tEventDetailedClick>(); });

auto reg_search_exps_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Search Exps"),
    [] { return new EventJSONParserForType<tEventSearchExps>(); });
auto reg_search_exps_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Search Exps"),
    [] { return new EventBinaryParserForType<tEventSearchExps>(); });

auto reg_search_top10_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Search", "Top 10"),
    [] { return new EventJSONParserForType<tEventSearchTop10>(); });
auto reg_search_top10_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Search", "Top 10"),
    [] { return new EventBinaryParserForType<tEventSearchTop10>(); });

}  // namespace event
}  // namespace logging

