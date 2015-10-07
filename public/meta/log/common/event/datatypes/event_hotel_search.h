// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

// this file contains all the events associated with the "Hotel Search" category

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_HOTEL_SEARCH_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_HOTEL_SEARCH_H_

#include "base/defs.h"
#include "meta/log/common/event/datatypes/canonical_objects.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

// helper structure that describes the price
struct tRateHelper {
  string source = "";
  float base = 0;
  float tot = 0;
  string rtype = "";
  string curr = "";
  SERIALIZE(source*1 / base*2 / tot*3 / rtype*4 / curr*5);
};

// helper structure that lists a few Price object
struct tLowestRates {
  tRateHelper lowest_known;
  tRateHelper second_lowest_known;
  SERIALIZE(lowest_known*1 / second_lowest_known*2);
};

struct tRateTypes {
  int mil = 0;
  int govt = 0;
  int senior = 0;
  int aaa = 0;
  SERIALIZE(mil*1 / govt*2 / senior*3 / aaa*4);
};

struct tFilters {
  bool is_default = true;
  unordered_set<string> applied;
  SERIALIZE(DEFAULT_CUSTOM /is_default*1 / applied*2);
};

////////////////////////////////////////////////////////////
// EVENT STRUCTS
////////////////////////////////////////////////////////////

// This struct is used for dated and dateless searches
struct tEventSearch {
  tSearchContext search_context;

  SERIALIZE(SERIALIZE_REQUIRED / search_context*1);
};

// represents the underlying json blob inside the request.value
// when the action "Load time" occcurs
struct tEventSearchLoadTime {
  int value = 0;

  SERIALIZE(SERIALIZE_REQUIRED / value*1);
};

struct tEventSearchImpression {
  tSearchContext search_context;
  vector<tImpressionContext> impressions;
  string view;

  SERIALIZE(SERIALIZE_REQUIRED / search_context*1 / impressions*2 / view*3);
};

struct tEventSearchHotelProfileClick {
  tSearchContext search_context;
  tHotel hotel;
  tBriefRate brief_rate;
  int current_pos = 0;
  string where_clicked = "";
  string view = "";
  int page = 0;

  SERIALIZE(SERIALIZE_REQUIRED / search_context*1 / hotel*2 / brief_rate*3 / current_pos*4 /
            where_clicked*5 / SERIALIZE_OPTIONAL / view*6 / page*7);
};

struct tEventSearchHomePageVisit {
  string default_search;

  SERIALIZE(SERIALIZE_OPTIONAL / default_search*1);
};

// TODO(otasevic): deprecate the events below

// represents the underlying json blob inside the request.value
// when the action "SearchExps" occcurs
struct tEventSearchExps {
  string city;
  string ctrseg;
  string dist;
  string mic;
  string session;
  int hid = 0;

  SERIALIZE(DEFAULT_CUSTOM / city*1 / ctrseg*2 / dist*3 /
            mic*4 / session*5 / hid*6);
};

// represents the underlying json blob inside the request.value
// when the action "Top 10" occcurs. It containst a few helper
// structs because the underlying json is cascaded
struct tEventSearchTop10 {
  // helper structure that lists pricing info for a specific hotel
  struct HotelPricingInfo {
    int hid = 0;
    tLowestRates pricing;
    int idx = 0;

    SERIALIZE(DEFAULT_CUSTOM / hid*1 / pricing*2 / idx*3);
  };

  string sort_type;
  vector<HotelPricingInfo> top;

  SERIALIZE(DEFAULT_CUSTOM / sort_type*1 / top*2);
};

struct tEventDetailedClick {
  struct {
    int n_impressions = 0;
    string sum_price_seen; // float as a string
    SERIALIZE(DEFAULT_CUSTOM / n_impressions*1 / sum_price_seen*2);
  } impression_stats;
  float global_average_price = 0;
  string ranker_id;
  int stars = 0;
  string sort_type;
  tRateHelper selected_rate;
  float distance = 0;
  tFilters filters;
  int hid = 0;
  int idx = 0;
  bool is_mobile = false;
  string name;
  string partner;
  tLowestRates pricing;
  SERIALIZE(DEFAULT_CUSTOM / impression_stats*1 / global_average_price*2 / ranker_id*3 /
            stars*4 / sort_type*5 / selected_rate*6 / distance*7 /
            filters*8 / hid*9 / idx*10 / is_mobile*11 / name*12 / partner*13 /
            pricing*14);
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_HOTEL_SEARCH_H_
