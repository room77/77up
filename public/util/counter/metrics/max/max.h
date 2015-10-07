// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_MAX_MAX_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_MAX_MAX_H_

#include <atomic>

#include "base/defs.h"
#include "util/counter/metrics/metric.h"
#include "util/counter/metrics/metric_datatypes.h"

namespace counter {
namespace metrics {

class Max : public MetricInterface {
 public:
  virtual ~Max() {}

  virtual operator float() const { return result_; }

  virtual void Update(const tCountedEvent& event, int num_events) {
    if (num_events > 0 && event.value > result_) {
      result_ =  event.value;
    }
  }

  virtual Max& operator+=(const MetricInterface& c) {
    if (c > result_) result_ = c;
    return *this;
  }

  // WARNING: -= operator does not make too much sense for max - unless all the events are kept
  // around we simply don't know how to recompute the minimum if only the value is kept
  virtual Max& operator-=(const MetricInterface& c) {
    return *this;
  }

  virtual void Reset() {
    result_ = numeric_limits<int>::min();
  }

 private:
  // Contains a minimum of the values of the processed events
  atomic<float> result_{numeric_limits<int>::min()};
};

}  // namespace metrics
}  // namespace counter


#endif  // _PUBLIC_UTIL_COUNTER_METRICS_MAX_MAX_H_
