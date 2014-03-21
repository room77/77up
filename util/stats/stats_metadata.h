// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_STATS_STATS_METADATA_H_
#define _UTIL_STATS_STATS_METADATA_H_

#include <limits>

#include "base/defs.h"
#include "util/serial/serializer.h"

namespace stats {

// Basic meta data that can be associated with a stat.
struct BasicMetaData {
  void Increment(double value = 1) {
    ++n;
    sum += value;
    squared_sum += value * value;

    if (min > value) min = value;
    if (max < value) max = value;
  }

  void clear() {
    n = 0;
    sum = 0;
    min = numeric_limits<int>::max();
    max = 0;
    squared_sum = 0;
  }

  fixedint<uint64_t> timestamp = 0;
  uint64_t n = 0;  // unweighted
  double sum = 0;  // weighted
  double min = numeric_limits<int>::max();  // The min value.
  double max = 0;  // The max value
  double squared_sum = 0;  // The squared sum.

  SERIALIZE(DEFAULT_CUSTOM / timestamp*1/ n*2 / sum*3 / min*4 / max*5 / squared_sum*6);

  // The timestamp from which new data should be read.
  // This field is not serialized and set at runtime.
  uint64_t read_ahead_from_time = 0;
};

}  // namespace stats


#endif  // _UTIL_STATS_STATS_METADATA_H_
