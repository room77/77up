// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_HOTEL_PROFILE_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_HOTEL_PROFILE_H_

#include "base/common.h"

#include "base/defs.h"
#include "meta/log/common/event/datatypes/canonical_objects.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

// visit structure that describes the data inside the value of for the (Hotel Profile, Visit) event
struct tEventHotelProfileVisit {
  tHotel hotel;

  SERIALIZE(SERIALIZE_REQUIRED / hotel*1);
};

}  // namespace event
}  // namespace logging

#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_HOTEL_PROFILE_H_
