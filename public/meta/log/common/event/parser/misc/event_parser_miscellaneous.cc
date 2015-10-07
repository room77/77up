// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)


#include "meta/log/common/event/datatypes/event_miscellaneous.h"
#include "meta/log/common/event/parser/event_parser.h"


namespace logging {
namespace event {

// registration of parsers associated with categories that have a single action (miscellaneous)
auto reg_home_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Home", "Home Experiment"),
    [] { return new EventJSONParserForType<tEventHomeExperiment>(); });
auto reg_home_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Home", "Home Experiment"),
    [] { return new EventBinaryParserForType<tEventHomeExperiment>(); });


auto reg_page_exit_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Page Exit", "Page Exit"),
    [] { return new EventJSONParserForType<tEventPageExit>(); });
auto reg_page_exit_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Page Exit", "Page Exit"),
    [] { return new EventBinaryParserForType<tEventPageExit>(); });

auto reg_new_visit_json_parser = EventParserInterface::bind(
    EventParserJSONKey("New Visit", "Referrer"),
    [] { return new EventJSONParserForType<tEventNewVisit>(); });
auto reg_new_visit_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("New Visit", "Referrer"),
    [] { return new EventBinaryParserForType<tEventNewVisit>(); });

}  // namespace event
}  // namespace logging

