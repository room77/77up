// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/counter.h"

#include <mutex>
#include <tuple>

namespace counter {

void Counter::ProcessEvent(const metrics::tCountedEvent& event, int num_events) {
  // wrap an event with metrics
  MetricsMap all_metrics = CreateMetricsMap(metric_config_);
  for (auto& kv : all_metrics) {
    kv.second->Update(event, num_events);
  }
  UpdateBins();
  PlaceMetricsInBin(event.timestamp, all_metrics, binsets_.begin());

  // Update the all time metrics if user specified that during construction of counter
  if (count_all_time_) {
    for (auto& kv : all_time_metrics_) {
      kv.second->Update(event, num_events);
    }
  }
}

Counter::MetricsMap Counter::GetMetricsForInterval(uint64_t interval_duration,
                                                   const MetricSet& metric_set) {
  // first update the bins to make sure that indices are correctly positioned
  UpdateBins();

  vector<BinSet>::iterator binsets_it = binsets_.begin();

  // total time that one bin set represents (ex. binset with 2 5-min bins is 10min (in microsec))
  uint64_t bin_set_length;

  MetricsMap metrics_result_map = CreateMetricsMap(metric_set);

  // iterate through a vector of binsets to find the one that corresponds to a given interval
  while (interval_duration > 0 && binsets_it != binsets_.end()) {
    // TODO(otasevic) this could possibly lead to overflow
    bin_set_length = binsets_it->binsize * binsets_it->bins.size();

    // if total time the binset represent is smaller than the interval duration, simply add total
    // metrics to the running metrics and continue to the next binset
    if (bin_set_length < interval_duration) {
      for (auto& metric : metrics_result_map) {
        // TODO(otasevic): replace this condition with bool method on metric
        if (metric.first != "min" && metric.first != "max") {
          *(metric.second) += (*(binsets_it->total_metrics.find(metric.first)->second));
        } else {
          // for some metrics, total_metrics cannot be kept correctly, so for them we need to sum
          // over all the bins in the binset
          for (MetricsMap& bin : binsets_it->bins) {
            *(metric.second) += *bin.find(metric.first)->second;
          }
        }
      }
      // TODO(otasevic): remove this check - not necessary here because already in if statement
      interval_duration = (
          interval_duration > bin_set_length) ? (interval_duration - bin_set_length) : 0;
      ++binsets_it;
    } else {
      // go into the bin set and iterate over the bins to find the proper one based on the interval
      int bins_index = binsets_it->last_index;
      bool made_full_circle = false;

      while (interval_duration > 0 && !made_full_circle) {
        for (auto& metric : metrics_result_map) {
          *(metric.second) += (*(binsets_it->bins.at(bins_index).find(metric.first)->second));
        }
        interval_duration = (
            interval_duration > binsets_it->binsize)
                ? (interval_duration - binsets_it->binsize) : 0;
        bins_index = (bins_index - 1 + binsets_it->bins.size()) % binsets_it->bins.size();
        if (bins_index == binsets_it->last_index) made_full_circle = true;
      }
    }
  }
  return metrics_result_map;
}

// This method assumes that bins were updated just before the call to this function
void Counter::PlaceMetricsInBin(uint64_t timestamp, const MetricsMap& metrics_map,
                                vector<BinSet>::iterator binsets_it) {
  uint64_t current_time = ::util::Timestamp::Now<chrono::microseconds>();
  // Compute how long ago an event happened (if event from the future, just ignore it).
  uint64_t interval_duration = (current_time > timestamp) ? (current_time - timestamp) : 0;

  // TODO(otasevic): duplicated code...consider...
  uint64_t bin_set_length;
  int bins_index = 0;

  while (interval_duration > 0 && binsets_it != binsets_.end()) {
    // TODO(otasevic) this could possibly lead to overflow
    bin_set_length = binsets_it->binsize * binsets_it->bins.size();
    if (bin_set_length < interval_duration) {
      interval_duration = interval_duration - bin_set_length;
      ++binsets_it;
    } else {
      interval_duration = (
          interval_duration > binsets_it->binsize) ? (interval_duration - binsets_it->binsize) : 0;
      bins_index = binsets_it->last_index;
      bool made_full_circle = false;
      // TODO(otasevic): potentially change this loop
      while (interval_duration > 0 && !made_full_circle) {
        interval_duration = (
          interval_duration > binsets_it->binsize)
              ? (interval_duration - binsets_it->binsize) : 0;
        bins_index = (bins_index - 1 + binsets_it->bins.size()) % binsets_it->bins.size();
        if (bins_index == binsets_it->last_index) made_full_circle = true;
      }
    }
  }

  if (binsets_it != binsets_.end()) {  // Check that interval is not outside of what is tracked.
    // add to individual bins
    for (auto& metric : binsets_it->bins.at(bins_index)) {
      metric.second->operator += (*(metrics_map.find(metric.first)->second));
    }
    // add to aggregate measures
    for (auto& metric : binsets_it->total_metrics) {
      metric.second->operator +=(*(metrics_map.find(metric.first)->second));
    }
  }
}

void Counter::UpdateBins() {
  uint64_t current_time = ::util::Timestamp::Now<chrono::microseconds>();

  // Element is a tuple specifying when event(s) happened (uint64_t), metrics on those events and
  // at which next binset the attempt to insert should be made.
  vector<tuple<uint64_t, MetricsMap, vector<BinSet>::iterator>> elements_to_move;

  for (vector<BinSet>::iterator it = binsets_.begin(); it != binsets_.end(); ++it) {
    int discretized_interval = (current_time > it->timestamp) ?
        ((current_time - it->timestamp) / it->binsize) : 0;
    // Update only if current time exceeds the last update time by more than a size of the bin.
    if (discretized_interval > 0) {
      // old_bins_index is an index of an element that needs to be moved up to the next bin set.
      int old_bins_index = it->last_index + 1;
      int old_bins_index_end = it->last_index + discretized_interval + 1;
      int modularized_old_bins_index;

      static std::mutex binset_lock;
      std::lock_guard<std::mutex> l(binset_lock);
      it->last_index = (it->last_index + discretized_interval) % it->bins.size();

      // Update the timestamp by a discretized time.
      it->timestamp += discretized_interval * it->binsize;
      // Move all the elements from old_bins_index to old_bins_index_end into a higher BinSet or
      // discard them if no such BinSet is present.
      for (; old_bins_index < old_bins_index_end; ++old_bins_index) {
        modularized_old_bins_index = old_bins_index % it->bins.size();
        MetricsMap m = CopyMetricsMap(it->bins.at(modularized_old_bins_index));
        int index_difference = (old_bins_index_end - 1) - old_bins_index;
        uint64_t bin_timestamp = (
            it->timestamp > (index_difference * it->binsize)) ?
                (it->timestamp - index_difference * it->binsize) : 0;

        // TODO(otasevic): does this do what I want? it+1 should be fine and should not leapfrog
        // because it is always strictly < .end() so it+1 will be .end() when I am exploring the
        // last element so PlaceMetricsInBin will recognize it and not even consider this move...
        elements_to_move.push_back(make_tuple(bin_timestamp, std::move(m), it+1));

        // Update aggregate metrics for the bin set.
        for (auto& metric : it->bins.at(modularized_old_bins_index)) {
          *(it->total_metrics.find(metric.first)->second) -= *(metric.second);
        }
        // Reset all the metrics that are being copied to another place.
        for (auto& kv : it->bins.at(modularized_old_bins_index)) {
          kv.second->Reset();
        }
      }
    }
  }
  // The function is done adjusting all the timestamps, move all the items that need to be moved.
  for (auto& el : elements_to_move) {
    PlaceMetricsInBin(get<0>(el), get<1>(el), get<2>(el));  // TODO(otasevic): change
  }
}

// An example of how counters with custom config can be created.
/*Counter::CounterTimeConfig time_config = {pair<uint64_t, int> (1 * 60 * 1000 * 1000, 2),
                                            pair<uint64_t, int> (5 * 60 * 1000 * 1000, 2)};

auto reg_minute_counter = CounterBase::bind("minute_counter",
    [] { return new Counter(time_config); });*/

}  // namespace counter
