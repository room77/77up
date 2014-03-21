// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/datatypes/event_monetized_click.h"
#include "meta/log/common/event/parser/event_parser.h"

namespace logging {
namespace event {

// registration of parsers associated with the "Monetized Clicks" category
auto reg_mclick_expanded_card_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Monetized Click", "Expanded Card"),
    [] { return new EventJSONParserForType<tEventMClickExpandedCard>(); });
auto reg_mclick_expanded_card_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Monetized Click", "Expanded Card"),
    [] { return new EventBinaryParserForType<tEventMClickExpandedCard>(); });

auto reg_mclick_hotel_profile_featured_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Monetized Click", "Hotel Profile Featured"),
    [] { return new EventJSONParserForType<tEventMClickHotelProfileFeatured>(); });
auto reg_mclick_hotel_profile_featured_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Monetized Click", "Hotel Profile Featured"),
    [] { return new EventBinaryParserForType<tEventMClickHotelProfileFeatured>(); });

auto reg_mclick_hotel_profile_rates_table_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Monetized Click", "Hotel Profile Rates Table"),
    [] { return new EventJSONParserForType<tEventMClickHotelProfileRatesTable>(); });
auto reg_mclick_hotel_profile_rates_table_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Monetized Click", "Hotel Profile Rates Table"),
    [] { return new EventBinaryParserForType<tEventMClickHotelProfileRatesTable>(); });

auto reg_mclick_serp_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Monetized Click", "Serp"),
    [] { return new EventJSONParserForType<tEventMClickSerp>(); });
auto reg_mclick_serp_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Monetized Click", "Serp"),
    [] { return new EventBinaryParserForType<tEventMClickSerp>(); });

auto reg_mclick_similar_hotel_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Monetized Click", "Similar Hotel"),
    [] { return new EventJSONParserForType<tEventMClickSimilarHotel>(); });
auto reg_mclick_similar_hotel_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Monetized Click", "Similar Hotel"),
    [] { return new EventBinaryParserForType<tEventMClickSimilarHotel>(); });

auto reg_mclick_sponsored_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Monetized Click", "Sponsored"),
    [] { return new EventJSONParserForType<tEventMClickSponsored>(); });
auto reg_mclick_sponsored_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Monetized Click", "Sponsored"),
    [] { return new EventBinaryParserForType<tEventMClickSponsored>(); });

}  // namespace event
}  // namespace logging

