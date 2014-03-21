// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_CANONICAL_OBJECTS_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_CANONICAL_OBJECTS_H_

#include "base/defs.h"
#include "util/serial/serializer.h"
#include "util/serial/types/arbit_blob.h"
#include "util/time/localtime.h"

namespace logging {
namespace event {

// helper structure that describes the hotel
struct tHotel {
  int hid = 0;
  string name = "";
  float stars = 0;
  string city = "";
  string state = "";
  string country = "";
  float user_rating = 0;

  SERIALIZE(SERIALIZE_REQUIRED / hid*1 / SERIALIZE_OPTIONAL / name*2 / stars*3 / city*4 / state*5 /
            country*6 / user_rating*7);
};

// rate canonical object
struct tRate {
  string desc = "";
  string url = "";
  string rate_type = "";
  bool prepaid = false;
  vector<string> merchs;
  string curr = "";
  bool conv = false; // TODO(otasevic): add a comment
  float base = 0;
  float tot = 0;
  bool bookable = false;
  string source = "";
  int srcid = 0;
  float first = 0;

  SERIALIZE(SERIALIZE_REQUIRED / desc*1 / url*2 / rate_type*3 / prepaid*4 / merchs*5 /
            curr*6 / conv*7 / base*8 / tot*9 / bookable*10 / source*11 / srcid*12 / first*13);
};

// rate canonical object
struct tBriefRate {
  string source = "";
  float base = 0;
  float tot = 0;
  string curr = "";
  bool conv = false;

  SERIALIZE(SERIALIZE_OPTIONAL / source*1 / base*2 / tot*3 / curr*4 / conv*5);
};

// Sponsored rate canonical object
struct tSponsoredRate {
  string curr = "";
  string desc = "";
  string url = "";
  int nstars = 0;
  string type = "";
  float rate = 0;

  SERIALIZE( SERIALIZE_REQUIRED / curr*1 / desc*2 / url*3 / SERIALIZE_OPTIONAL / nstars*4 / type*5 /
             rate*6 );
};

struct tSponsoredAd {
  vector<tSponsoredRate> rates;
  string desc;
  int pos;
  string source;
  string url;

  SERIALIZE(SERIALIZE_REQUIRED / rates*1 / desc*2 / pos*3 / source*4 / url*5);
};

// sort canonical object
struct tSort {
  string id = "";
  serial::types::ArbitBlob params;

  SERIALIZE(SERIALIZE_REQUIRED / id*1 / SERIALIZE_OPTIONAL / params*2);
};

struct tSearch {
  double lat = 0;
  double lon = 0;
  int rooms = 0;
  int guests = 0;

  // Suggest id: used for autocomplete.
  string sgst_id;

  string key;  // The search string.
  string currency;
  bool dateless = false;

  tHotel hotel;
  LocalDate checkin;
  LocalDate checkout;

  int days_until_checkin = 0;  // convenience parameter - number of days from search to check-in
  int num_nights = 0;  // convenience parameter for number of nights staying

  SERIALIZE(SERIALIZE_REQUIRED / lat*1 / lon*2 / rooms*3 / guests*4 / sgst_id*5 /
            key*6 / currency*7 / dateless*8 / SERIALIZE_OPTIONAL / hotel*9 / checkin*10 /
            checkout*11 / days_until_checkin*12 / num_nights*13);
};

struct tFilter {
  string id = "";
  bool on = false;
  serial::types::ArbitBlob params;

  SERIALIZE(SERIALIZE_REQUIRED / id*1 / on*2 / SERIALIZE_OPTIONAL / params*3);
};

struct tFiltersState {
  tFilter amenities;
  tFilter price_max;
  tFilter chains;
  tFilter neighborhoods;
  tFilter star_min;
  tFilter special_rates;

  SERIALIZE(SERIALIZE_OPTIONAL / amenities*1 / price_max*2 / chains*3 / neighborhoods*4 /
            star_min*5 / special_rates*6);
};

// search context canonical object
struct tSearchContext {
  tSearch search;
  serial::types::ArbitBlob all_filters;
  tSort sort;
  unordered_map<string, unordered_map<string, string> > search_expts;

  SERIALIZE(SERIALIZE_REQUIRED / search*1 / all_filters*2 / sort*3 / search_expts*4);
};

struct tImpressionContext {
  tHotel hotel;
  tBriefRate brief_rate;
  int position = -1;  // default position is -1 in order to avoid consusion with 0-th position

  SERIALIZE(SERIALIZE_REQUIRED / hotel*1 / brief_rate*2 / SERIALIZE_OPTIONAL / position*3);
};

struct tBookParams {
  float base = 0;
  LocalDate check_in;
  LocalDate check_out;
  int hid = 0;
  int guests = 0;
  float tot = 0;
  string curr = "";

  SERIALIZE(SERIALIZE_REQUIRED / base*1 / check_in*2 / check_out*3 / hid*4 /
            guests*5 / SERIALIZE_OPTIONAL / curr*6 / tot*7);
};

}  // namespace event
}  // namespace logging

#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_CANONICAL_OBJECTS_H_
