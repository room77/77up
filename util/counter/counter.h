// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_COUNTER_H_
#define _PUBLIC_UTIL_COUNTER_COUNTER_H_

#include <map>
#include <memory>
#include <utility>

#include "base/defs.h"
#include "util/counter/counter_base.h"
#include "util/counter/metrics/metric.h"
#include "util/counter/metrics/metric_datatypes.h"
#include "util/time/timestamp.h"

namespace counter {

// This class implements a way to keep track of specific events and retrieve
// information about what happened in the last X (minutes, hours, etc.).
class Counter : public CounterBase {
 public:
  virtual ~Counter() {}

  struct BinSet {
    uint64_t binsize;          // ex. 1 min bin
    vector<MetricsMap> bins;   // a vector of Metrics
    uint64_t timestamp;        // the time when the last update happened
    int last_index;            // the index in the vector where last element was inserted
    MetricsMap total_metrics;  // metrics across the entire bin set
  };

  // This type defines a config type for Counter - an optional parameter that lets users of
  // Counter class specify the kind of bucketing they want. This type simply consists of
  // pairs of (time interval length, number of buckets for that interval).
  typedef vector<pair<uint64_t, int>> CounterTimeConfig;

  // This is a constructor that has two optional arguments. The first one specifies the
  // kind of bins that are used (1 min, 1 hour etc.) and the number of such bins.
  // The second argument specifies the kind of metrics that are tracked.

  // WARNING: Because this class' role is to count events in memory, the
  // total number of bins you decide to use will determine the amount of memory this class ends
  // up using. Please be very careful if you are changing the default time_config.
  // Default config gives 60 1-sec bins, 60 1-minute bins, 24 1-hour bins
  // and 30 1-day bins. (in microseconds)
  // TODO(otasevic) these should be duration objects
  Counter(const CounterTimeConfig& time_config = {pair<uint64_t, int> (1000 * 1000ULL, 60),
      pair<uint64_t, int> (60 * 1000 * 1000ULL, 60),
      pair<uint64_t, int> (60 * 60 * 1000 * 1000ULL, 24),
      pair<uint64_t, int> (24 * 60 * 60 * 1000 * 1000ULL, 30)},
          const MetricSet& metric_config = {"count"},
          const bool count_all_time = false)
      : metric_config_(metric_config), count_all_time_(count_all_time) {
    Initialize(time_config);
  }

  // Handles the event counting as specified through the config parameters in the constructor.
  virtual void ProcessEvent(const metrics::tCountedEvent& event, int num_events = 1);

  // This function retrieves a set of metric for the events that happened within that last
  // interval_duration (in microseconds). The metric_set parameter is optional and if left out it
  // defaults to the entire set of metrics with which the counter was initialized.
  // TODO(otasevic) write a function for default intervals that is much more efficient
  // TODO(otasevic) this is currently not constrained so people could potentially ask for 10 min
  // and get only 3 min worth of stuff just because of what is specified in config
  virtual MetricsMap GetMetricsForInterval(uint64_t interval_duration) {
    return GetMetricsForInterval(interval_duration, metric_config_);
  }
  virtual MetricsMap GetMetricsForInterval(uint64_t interval_duration, const MetricSet& metric_set);

  virtual MetricsMap GetAllTimeMetrics() {
    MetricsMap metrics_result_map = CopyMetricsMap(all_time_metrics_);
    return metrics_result_map;
  }

  // Getter methods
  const vector<BinSet>& GetBinsets() {
    return binsets_;
  }
  const MetricSet& GetMetricConfig() {
    return metric_config_;
  }

 protected:
  void Initialize(const CounterTimeConfig& time_config) {
    uint64_t total_time = 0;
    for (const pair<uint64_t, int>& bin_set_config : time_config) {
      BinSet new_bin_set;
      new_bin_set.bins.reserve(bin_set_config.second);
      for (int i = 0; i < bin_set_config.second; ++i) {
        new_bin_set.bins.push_back(std::move(CreateMetricsMap(metric_config_)));
      }
      new_bin_set.binsize = bin_set_config.first;
      new_bin_set.timestamp = ::util::Timestamp::Now<chrono::microseconds>() + total_time;
      new_bin_set.last_index = 0;
      new_bin_set.total_metrics = CreateMetricsMap(metric_config_);
      total_time += new_bin_set.binsize * new_bin_set.bins.size();
      binsets_.push_back(std::move(new_bin_set));
    }
    if (count_all_time_) {  // create this only if count_all_time is set to true
      all_time_metrics_ = CreateMetricsMap(metric_config_);
    }
  }

  // This method finds the bin for which the count needs to be increased
  // it starts from the beginning of the bin set vector and increases the
  // count based on the difference between current time and the segment_time...
  // ex: 2min<curent_time-segment_time<3min -> it goes into 3rd bin
  void PlaceMetricsInBin(uint64_t timestamp, const MetricsMap& metrics_map,
                         vector<BinSet>::iterator binset_index);

  // this method is called by every public method of this class to ensure that updates are done
  // just before any information is retrieved or any new information is added. In the method, we
  // look at all the bins and their timestamps and ensure that the pointer to the bin where the
  // next element needs to be inserted is updated. We also move all the elements that do not
  // belong to a particular set of bins to the next ("higher") set of bins if such exists or remove
  // them if such a set does not exist.
  void UpdateBins();

  // This method creates a map from metric name(string) to a unique proxy for that metric.
  // The metrics are created from the intersection of names given in the metric_set and names
  // in the metric_config_.
  MetricsMap CreateMetricsMap(const MetricSet& metric_set) {
    MetricsMap metrics;
    for (const string& metric_name : metric_set) {
      if (find(metric_config_.begin(), metric_config_.end(), metric_name) != metric_config_.end()) {
        auto new_proxy = metrics::MetricInterface::make_unique(metric_name);
        metrics.insert(make_pair(metric_name, smart_ptr_type(new_proxy.get())));
        new_proxy.release();
      }
    }
    return metrics;
  }

  // utility method to copy Metrics
  MetricsMap CopyMetricsMap(const MetricsMap& m) {
    MetricsMap metrics;
    for (auto& kv : m) {
      auto new_proxy = metrics::MetricInterface::make_unique(kv.first);
      *(new_proxy) += *(kv.second);
      metrics.insert(make_pair(kv.first, smart_ptr_type(new_proxy.get())));
      new_proxy.release();
    }
    return metrics;
  }

 private:
  // This is a vector of all the bin sets populated based on the config parameter in the constructor
  vector<BinSet> binsets_;
  const MetricSet metric_config_;
  bool count_all_time_;
  MetricsMap all_time_metrics_;  // This will be non-empty only if count_all_time_ is set to true
};

}  // namespace counter

#endif  // _PUBLIC_UTIL_COUNTER_COUNTER_H_
