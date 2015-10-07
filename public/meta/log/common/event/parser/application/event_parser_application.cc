// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/datatypes/event_application.h"
#include "meta/log/common/event/parser/event_parser.h"

namespace logging {
namespace event {

// registration of parsers associated with the "Hotel Search" category
auto reg_application_new_visit_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Application", "New Visit"),
    [] { return new EventJSONParserForType<tEventApplicationNewVisit>(); });
auto reg_application_new_visit_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Application", "New Visit"),
    [] { return new EventBinaryParserForType<tEventApplicationNewVisit>(); });

auto reg_application_uncaught_exception_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Application", "Uncaught Exception"),
    [] { return new EventJSONParserForType<tEventApplicationUncaughtException>(); });
auto reg_application_uncaught_exception_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Application", "Uncaught Exception"),
    [] { return new EventBinaryParserForType<tEventApplicationUncaughtException>(); });

auto reg_application_angular_exception_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Application", "AngularJS Exception"),
    [] { return new EventJSONParserForType<tEventApplicationAngularException>(); });
auto reg_capplication_angular_exception_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Application", "AngularJS Exception"),
    [] { return new EventBinaryParserForType<tEventApplicationAngularException>(); });

}  // namespace event
}  // namespace logging

