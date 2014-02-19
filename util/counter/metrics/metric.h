// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_METRIC_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_METRIC_H_

#include "base/defs.h"
#include "util/factory/factory.h"
#include "util/counter/metrics/metric_datatypes.h"

namespace counter {
namespace metrics {
// base metric
class MetricInterface : public Factory<MetricInterface, string> {
 public:
  virtual ~MetricInterface() {}

  virtual operator float() const = 0;

  virtual void Update(const tCountedEvent& event, int number_of_events) = 0;

  virtual MetricInterface& operator+=(const MetricInterface& metric) = 0;

  virtual MetricInterface& operator-=(const MetricInterface& metric) = 0;

  virtual void Reset() = 0;

  virtual int GetNumElements() const {
    ASSERT(0);  // the method is only implemented for certain derived classes
    return 0;
  }
};

}  // namespace metrics
}  // namespace counter

#endif  // _PUBLIC_UTIL_COUNTER_METRICS_METRIC_H_
