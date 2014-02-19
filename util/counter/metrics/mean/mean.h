// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_MEAN_MEAN_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_MEAN_MEAN_H_

#include <mutex>

#include "base/defs.h"
#include "util/counter/metrics/metric.h"
#include "util/counter/metrics/metric_datatypes.h"

namespace counter {
namespace metrics {

class Mean : public MetricInterface {
 public:
  virtual ~Mean() {}

  virtual operator float() const { return result_; }

  virtual void Update(const tCountedEvent& event, int num_events) {
    if (num_events > 0) {
      static std::mutex mean;
      std::lock_guard<std::mutex> l(mean);

      result_ = (result_ * num_elements_ + num_events * event.value) / (num_elements_ + num_events);
      num_elements_ += num_events;
    }
  }

  virtual Mean& operator+=(const MetricInterface& c) {
    static std::mutex mean;
    std::lock_guard<std::mutex> l(mean);

    int c_num_elements = c.GetNumElements();

    result_ = (result_ * num_elements_ + c_num_elements * c) / (num_elements_ + c_num_elements);
    return *this;
  }

  virtual Mean& operator-=(const MetricInterface& c) {
    static std::mutex mean;
    std::lock_guard<std::mutex> l(mean);

    int c_num_elements = c.GetNumElements();

    if (num_elements_ -  c_num_elements > 0) {
      result_ = (result_ * num_elements_ - c_num_elements * c) / (num_elements_ - c_num_elements);
    } else {
      result_ = 0;
    }
    return *this;
  }

  virtual void Reset() {
    static std::mutex mean;
    std::lock_guard<std::mutex> l(mean);
    result_ = 0;
    num_elements_ = 0;
  }

  virtual int GetNumElements() const { return num_elements_; }

 private:
  float result_ = 0;  // Represents a running mean of the values of the processed events
  int num_elements_ = 0;  // TODO(otasevic): this is kind of redundant with the count metric...
};

}  // namespace metrics
}  // namespace counter


#endif  // _PUBLIC_UTIL_COUNTER_METRICS_MEAN_MEAN_H_
