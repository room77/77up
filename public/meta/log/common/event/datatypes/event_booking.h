// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_BOOKING_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_BOOKING_H_

#include "base/defs.h"
#include "meta/log/common/event/datatypes/canonical_objects.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

////////////////////////////////////////////////////////////
// EVENT STRUCTS
////////////////////////////////////////////////////////////

struct tEventBookingVisit {
  tBookParams book_params;

  SERIALIZE(SERIALIZE_REQUIRED / book_params*1);
};

struct tEventBookButton {
  bool valid = false;

  SERIALIZE(SERIALIZE_REQUIRED / valid*1);
};

// TODO(otasevic): this event should be restructured, some info may not be required
struct tEventBookingConfirmationDetails {
  string conf_id = "";
  float total = 0;
  float tax = 0;
  float base = 0;
  string currency = "";
  string city = "";
  string state = "";
  string country = "";
  string source = "";
  int hid = 0;
  int sid = 0;
  string hotel_name = "";
  string category = "";
  float total_per_room_before_tax = 0;
  int rooms = 0;
  bool is_test = false;

  SERIALIZE(SERIALIZE_REQUIRED / conf_id*1 / total*2 / tax*3 / base*4 / currency*5 / city*6 /
            state*7 / country*8 / source*9 / hid*10 / sid*11 / hotel_name*12 / category*13 /
            total_per_room_before_tax*14 / rooms*15 / is_test*16);
};

// TODO(otasevic): this event should be restructured, some info may not be required
struct tEventBookingConfirmed {
  string label = "";  // conf_id
  float val = 0;  // profit_margin - TODO(otasevic): not clear what this is - investigate!

  SERIALIZE(SERIALIZE_REQUIRED / label*1 / val*2);
};

struct tEventConfirmationPageVisit {
  string conf_id = "";

  SERIALIZE(SERIALIZE_REQUIRED / conf_id*1);
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_BOOKING_H_
