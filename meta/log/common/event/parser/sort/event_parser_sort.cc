// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/datatypes/event_sort.h"
#include "meta/log/common/event/parser/event_parser.h"

namespace logging {
namespace event {

// registration of parsers associated with the "Sort" category
auto reg_sort_click_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Sort", "click"),
    [] { return new EventJSONParserForType<tEventSortClick>(); });
auto reg_sort_click_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Sort", "click"),
    [] { return new EventBinaryParserForType<tEventSortClick>(); });

}  // namespace event
}  // namespace logging

