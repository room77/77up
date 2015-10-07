// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_MIN_MIN_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_MIN_MIN_H_

#include <atomic>

#include "base/defs.h"
#include "util/counter/metrics/metric.h"
#include "util/counter/metrics/metric_datatypes.h"

namespace counter {
namespace metrics {

class Min : public MetricInterface {
 public:
  virtual ~Min() {}

  virtual operator float() const { return result_; }

  virtual void Update(const tCountedEvent& event, int num_events) {
    if (num_events > 0 && event.value < result_) {
      result_ =  event.value;
    }
  }

  virtual Min& operator+=(const MetricInterface& c) {
    if (c < result_) result_ = c;
    return *this;
  }

  // WARNING: -= operator does not make too much sense for min - unless all the events are kept
  // around we simply don't know how to recompute the minimum if only the value is kept
  virtual Min& operator-=(const MetricInterface& c) {
    return *this;
  }

  virtual void Reset() {
    result_ = numeric_limits<float>::max();
  }

 private:
  // Contains a minimum of the values of the processed events
  atomic<float> result_{numeric_limits<float>::max()};
};

}  // namespace metrics
}  // namespace counter


#endif  // _PUBLIC_UTIL_COUNTER_METRICS_MIN_MIN_H_
