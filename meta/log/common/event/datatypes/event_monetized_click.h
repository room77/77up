// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_MONETIZED_CLICK_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_MONETIZED_CLICK_H_

#include "base/defs.h"
#include "meta/log/common/event/datatypes/canonical_objects.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

////////////////////////////////////////////////////////////
// EVENT STRUCTS
////////////////////////////////////////////////////////////

// This struct is used for the monetized click event on the expanded card on serp
struct tEventMClickExpandedCard {
  tRate rate;
  tHotel hotel;
  tSearchContext search_context;
  int position = 0;

  SERIALIZE(SERIALIZE_REQUIRED / rate*1 / hotel*2 / search_context*3 / position*4);
};

// This struct is used for the click event on the book button on the top of hotel profile page
struct tEventMClickHotelProfileFeatured {
  tRate rate;
  tHotel hotel;
  tSearchContext search_context;

  bool all_rates = false;

  // The following three fields describe quick links section bellow the book button
  int num_additional_rates = 0;
  float range_min = 0;
  float range_max = 0;

  // Position of the hotel on the serp
  int serp_position = 0;

  bool map = false;

  SERIALIZE(SERIALIZE_REQUIRED / rate*1 / hotel*2 / search_context*3 /
            SERIALIZE_OPTIONAL / all_rates*4 / num_additional_rates*5 / range_min*6 / range_max*7 /
            serp_position*8 / map*9);
};

// This struct is used for the click event on the book button
// in the rates table on the hotel profile page
struct tEventMClickHotelProfileRatesTable {
  tRate rate;
  tHotel hotel;

  // Position of a particular rate in a specific source. For example, if Expedia has 3 rates,
  // their position_within_source would be 0,1,2, respecitively, regardless of how Expedia
  // ranks against other sources
  int position_within_source = 0;

  // Position of the source grouping in the rates table. For example, if in order the lowest rates
  // are from Expedia, Otels and Priceline, all Priceline rates will have a source_position of 2
  int source_position = 0;

  bool all_rates_shown = false;  // Indicates whether all rates from all the sources are shown
  tSearchContext search_context;
  int num_sources = 0;  // Number of sources in the rates table
  string where_clicked = "";  // describes the place on the rate's card where the user clicked

  // Number of rates from source (this might be more than the number showing, if all_rates is false)
  int num_rates_from_source = 0;

  SERIALIZE(SERIALIZE_REQUIRED / rate*1 / hotel*2 / source_position*3 / all_rates_shown*4 /
            search_context*5 / num_sources*6 / where_clicked*7 / SERIALIZE_OPTIONAL /
            position_within_source*8 / num_rates_from_source*9);
};

// This struct is used for the click event on the book button on the serp
struct tEventMClickSerp {
  tRate rate;
  tHotel hotel;

  // Position of the hotel on the serp
  int position = 0;

  tSearchContext search_context;

  // The following three fields describe quick links section bellow the book button
  int num_additional_rates = 0;
  float range_min = 0;
  float range_max = 0;

  SERIALIZE(SERIALIZE_REQUIRED / rate*1 / hotel*2 / position*3 / search_context*4 /
            SERIALIZE_OPTIONAL / num_additional_rates*5 / range_min*6 / range_max*7);
};

// This struct is used for the click event on the book button on the top of hotel profile page
struct tEventMClickSimilarHotel {
  tRate rate;
  tHotel hotel;
  tSearchContext search_context;
  tHotel linked_from;  // hotel on whose hotel profile page the click happened
  int position_similar;  // position of the hotel in the list of similar hotels

  // The following three fields describe quick links section bellow the book button
  int num_additional_rates = 0;
  float range_min = 0;
  float range_max = 0;

  SERIALIZE(SERIALIZE_REQUIRED / rate*1 / hotel*2 / search_context*3 / linked_from*4 /
            position_similar*5 / SERIALIZE_OPTIONAL / num_additional_rates*6 / range_min*7 /
            range_max*8);
};

// This struct is used for the click on the sponsored card on the serp
struct tEventMClickSponsored {
  tSponsoredAd sponsored_ad;
  string where_clicked = "";
  tSearchContext search_context;
  string view = "";

  SERIALIZE(SERIALIZE_REQUIRED / sponsored_ad*1 / where_clicked*2 / search_context*3 / view*4);
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_MONETIZED_CLICK_H_
