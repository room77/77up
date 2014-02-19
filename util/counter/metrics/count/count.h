// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_COUNT_COUNT_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_COUNT_COUNT_H_

#include <atomic>

#include "base/defs.h"
#include "util/counter/metrics/metric.h"
#include "util/counter/metrics/metric_datatypes.h"

namespace counter {
namespace metrics {

class Count : public MetricInterface {
 public:
  virtual ~Count() {}

  virtual operator float() const { return result_; }

  virtual void Update(const tCountedEvent& event, int num_events) {
    if (num_events > 0) result_ += num_events;
  }

  virtual Count& operator+=(const MetricInterface& c) {
    result_ += c;
    return *this;
  }

  virtual Count& operator-=(const MetricInterface& c) {
    result_ -= c;
    if (result_ < 0) result_ = 0;
    return *this;
  }

  virtual void Reset() { result_ = 0; }

 private:
  atomic_int result_{0};  // contains a number of counted events
};

}  // namespace metrics
}  // namespace counter


#endif  // _PUBLIC_UTIL_COUNTER_METRICS_COUNT_COUNT_H_
