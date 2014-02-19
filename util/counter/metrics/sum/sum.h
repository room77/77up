// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_SUM_SUM_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_SUM_SUM_H_

#include <atomic>

#include "base/defs.h"
#include "util/counter/metrics/metric.h"
#include "util/counter/metrics/metric_datatypes.h"

namespace counter {
namespace metrics {

class Sum : public MetricInterface {
 public:
  virtual ~Sum() {}

  virtual operator float() const { return result_; }

  virtual void Update(const tCountedEvent& event, int num_events) {
    if (num_events > 0) result_ =  result_ + num_events * event.value;
  }

  virtual Sum& operator+=(const MetricInterface& c) {
    result_ = result_ + c;
    return *this;
  }

  virtual Sum& operator-=(const MetricInterface& c) {
    result_ = result_ - c;
    if (result_ < 0) result_ = 0.0f;
    return *this;
  }

  virtual void Reset() {
    result_ = 0.0f;
  }

 private:
  atomic<float> result_{0};  // Contains a sum of the values of the processed events
};

}  // namespace metrics
}  // namespace counter


#endif  // _PUBLIC_UTIL_COUNTER_METRICS_SUM_SUM_H_
