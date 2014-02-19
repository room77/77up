// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_METRIC_DATATYPES_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_METRIC_DATATYPES_H_

#include "base/defs.h"
#include "util/time/timestamp.h"

namespace counter {
namespace metrics {

// event that is passed to a counter
struct tCountedEvent {
  uint64_t timestamp = ::util::Timestamp::Now<chrono::microseconds>();  // when the event occured
  float value = 1;  //  value associated with the event
};

}  // namespace metrics
}  // namespace counter

#endif  // _PUBLIC_UTIL_COUNTER_METRICS_METRIC_DATATYPES_H_
