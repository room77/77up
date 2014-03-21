// Copyright 2013 Room77, Inc.
// Author: vkasera@room77.com (Vishal Kasera)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_CONSTANTS_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_CONSTANTS_H_

#include "util/init/init.h"

namespace logging {
namespace event {

// Constants for validating Dated Search.
const double kLatitudeMin = -90.0;
const double kLatitudeMax = 90.0;
const double kLongitudeMin = -180.0;
const double kLongitudeMax = 180.0;

// Defines a set of (category, action) pairs that are critical
// Critical means that these push will not happen unless all of these events are being logged
// properly (passing all the checks in the validators)
const unordered_set<pair<string, string> > critical_events ({
    pair<string, string> ("Hotel Search", "Dated Search"),
    pair<string, string> ("Hotel Search", "Dateless Search"),
    pair<string, string> ("Hotel Search", "Home Page Visit"),
    pair<string, string> ("Hotel Search", "Hotel Profile Click"),
    pair<string, string> ("Monetized Click", "Hotel Profile Rates Table"),
    pair<string, string> ("Monetized Click", "Serp"),
    pair<string, string> ("Monetized Click", "Sponsored"),
    pair<string, string> ("Monetized Click", "Hotel Profile Featured"),
    pair<string, string> ("Monetized Click", "Similar Hotel"),
    pair<string, string> ("Booking", "Book button"),
    pair<string, string> ("Booking", "Visit"),
    pair<string, string> ("Booking", "Confirmation Page Visit"),
    pair<string, string> ("Hotel Profile", "Visit"),
    pair<string, string> ("Application", "New Visit"),
    pair<string, string> ("Application", "Uncaught Error"),
    pair<string, string> ("Application", "Angular JS Exception")
    } );

}  // namespace event
}  // namespace logging

#endif  // _PUBLIC_META_LOG_COMMON_EVENT_CONSTANTS_H_
