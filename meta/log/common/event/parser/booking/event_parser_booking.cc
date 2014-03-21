// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/datatypes/event_booking.h"
#include "meta/log/common/event/parser/event_parser.h"

namespace logging {
namespace event {

// registration of parsers associated with the "Hotel Search" category
auto reg_booking_visit_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Booking", "Visit"),
    [] { return new EventJSONParserForType<tEventBookingVisit>(); });
auto reg_booking_visit_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Booking", "Visit"),
    [] { return new EventBinaryParserForType<tEventBookingVisit>(); });

auto reg_book_button_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Booking", "Book button"),
    [] { return new EventJSONParserForType<tEventBookButton>(); });
auto reg_book_button_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Booking", "Book button"),
    [] { return new EventBinaryParserForType<tEventBookButton>(); });

auto reg_confirmation_page_visit_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Booking", "Confirmation Page Visit"),
    [] { return new EventJSONParserForType<tEventConfirmationPageVisit>(); });
auto reg_confirmation_page_visit_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Booking", "Confirmation Page Visit"),
    [] { return new EventBinaryParserForType<tEventConfirmationPageVisit>(); });

auto reg_booking_confirmed_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Booking", "Confirmed"),
    [] { return new EventJSONParserForType<tEventBookingConfirmed>(); });
auto reg_booking_confirmed_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Booking", "Confirmed"),
    [] { return new EventBinaryParserForType<tEventBookingConfirmed>(); });

auto reg_booking_confirmation_details_json_parser = EventParserInterface::bind(
    EventParserJSONKey("Booking", "ConfirmationDetails"),
    [] { return new EventJSONParserForType<tEventBookingConfirmationDetails>(); });
auto reg_booking_confirmation_details_binary_parser = EventParserInterface::bind(
    EventParserBinaryKey("Booking", "ConfirmationDetails"),
    [] { return new EventBinaryParserForType<tEventBookingConfirmationDetails>(); });

}  // namespace event
}  // namespace logging

