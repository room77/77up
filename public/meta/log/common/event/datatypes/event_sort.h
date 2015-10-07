// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// This file contains all the events associated with the "Sort" category.

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_SORT_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_SORT_H_

#include "base/defs.h"
#include "meta/log/common/event/datatypes/canonical_objects.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

////////////////////////////////////////////////////////////
// EVENT STRUCTS
////////////////////////////////////////////////////////////

// Actions
// 1. "Price (low to high) Click".
// 2. "Price (high to low) Click"
// 3. "Recommended Click" / "Popular Click"
// 4. "Distance Click"
// 5. "Star Rating Click"
// 6. "User Rating Click"
// 7. "Price Click"
// 7. "Reviews Click"
struct tEventSortClick {
  // The sort object associated with the event.
  tSort sort;

  SERIALIZE(SERIALIZE_REQUIRED / sort*1);

};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_SORT_H_
