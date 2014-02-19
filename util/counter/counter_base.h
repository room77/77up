// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_COUNTER_BASE_H_
#define _PUBLIC_UTIL_COUNTER_COUNTER_BASE_H_

#include <map>

#include "base/defs.h"
#include "util/factory/factory.h"
#include "util/counter/metrics/metric.h"
#include "util/counter/metrics/metric_datatypes.h"

namespace counter {

// This is a base class for the counter implementation.
class CounterBase : public Factory<CounterBase> {
 public:
  virtual ~CounterBase() {}

  // Defines a map from an enum type to a specific metric.
  // TODO(otasevic,oztekin): use unique_ptr once we resolve the compilation
  // issues with gcc 4.9.
  typedef shared_ptr<metrics::MetricInterface> smart_ptr_type;
  typedef map<string, smart_ptr_type> MetricsMap;

  // Defines a set of metrics. This is used to specify the kind of metrics to be tracked
  // and/or retrieved.
  typedef vector<string> MetricSet;

  // This method is implemented by every derived class and is supposed to update all the metrics
  // with the event parameters.
  virtual void ProcessEvent(const metrics::tCountedEvent& event, int num_events = 1) = 0;

  // This method retrieves all the registered metrics for a specific time interval.
  virtual MetricsMap GetMetricsForInterval(uint64_t interval_duration) = 0;

  virtual MetricsMap GetMetricsForInterval(uint64_t interval_duration,
                                           const MetricSet& metric_set) = 0;

  virtual MetricsMap GetAllTimeMetrics() = 0;
};

}  // namespace counter

#endif  // _PUBLIC_UTIL_COUNTER_COUNTER_BASE_H_
