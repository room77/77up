// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/datatypes/event_hotel_profile.h"
#include "meta/log/common/event/parser/event_parser.h"

namespace logging {
namespace event {

// registration of parsers associated with the "Hotel Profile" category
auto reg_profile_visit_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Hotel Profile", "Visit"),
    [] { return new EventJSONParserForType<tEventHotelProfileVisit>(); });
auto reg_profile_visit_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Hotel Profile", "Visit"),
    [] { return new EventBinaryParserForType<tEventHotelProfileVisit>(); });

}  // namespace event
}  // namespace logging

