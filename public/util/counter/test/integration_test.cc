// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include <memory>
#include <utility>

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/counter.h"
#include "util/counter/counter_base.h"
#include "util/counter/metrics/all_metrics.h"

namespace counter {
namespace test {

TEST(ProcessEvent, Sanity) {
  Counter::CounterTimeConfig test_time_config = {pair<uint64_t, int> (5000, 2), // 2 5-ms
                                                 pair<uint64_t, int> (10000, 2)}; // 2 10-ms

  Counter::MetricSet metric_set(1, "count");

  unique_ptr<Counter> counter(new Counter(test_time_config, metric_set));


  // sequence: 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1
  metrics::tCountedEvent event1;
  counter->ProcessEvent(event1);
  std::chrono::milliseconds dur1(8);
  std::this_thread::sleep_for(dur1);
  metrics::tCountedEvent event2;
  counter->ProcessEvent(event2, 2);
  std::chrono::milliseconds dur2(9);
  std::this_thread::sleep_for(dur2);
  metrics::tCountedEvent event3;
  counter->ProcessEvent(event3);


  const vector<Counter::BinSet>& binsets = counter->GetBinsets();

  EXPECT_EQ(2, binsets.size());

  EXPECT_EQ(1, binsets.at(0).last_index);
  EXPECT_EQ(0, binsets.at(1).last_index);

  EXPECT_EQ(0, static_cast<int>(*binsets.at(0).bins.at(0).find("count")->second));
  EXPECT_EQ(1, static_cast<int>(*binsets.at(0).bins.at(1).find("count")->second));

  EXPECT_EQ(3, static_cast<int>(*binsets.at(1).bins.at(0).find("count")->second));
  EXPECT_EQ(0, static_cast<int>(*binsets.at(1).bins.at(1).find("count")->second));
}

TEST(ProcessEvent, NoBins) {
  Counter::CounterTimeConfig test_time_config = {};
  Counter::MetricSet metric_set(1, "count");

  unique_ptr<Counter> counter(new Counter(test_time_config, metric_set));

  metrics::tCountedEvent event1;
  counter->ProcessEvent(event1);

  const vector<Counter::BinSet>& binsets = counter->GetBinsets();

  EXPECT_EQ(0, binsets.size());
}

TEST(ProcessEvent, AllTimeCount) {
  Counter::CounterTimeConfig test_time_config = {};
  Counter::MetricSet metric_set(1, "count");

  unique_ptr<Counter> counter(new Counter(test_time_config, metric_set, true));

  metrics::tCountedEvent event1;
  counter->ProcessEvent(event1);

  CounterBase::MetricsMap metrics_map = counter->GetAllTimeMetrics();

  EXPECT_EQ(1, *metrics_map.find("count")->second);
}

TEST(ProcessEvent, EmptyAllTimeCount) {
  Counter::CounterTimeConfig test_time_config = {};
  Counter::MetricSet metric_set(1, "count");

  unique_ptr<Counter> counter(new Counter(test_time_config, metric_set));

  metrics::tCountedEvent event1;
  counter->ProcessEvent(event1);

  CounterBase::MetricsMap metrics_map = counter->GetAllTimeMetrics();

  EXPECT_TRUE(metrics_map.empty());
}

TEST(ProcessEvent, DroppingEvent) {
  Counter::CounterTimeConfig test_time_config = {pair<uint64_t, int> (5000, 1), // 1 5-ms
                                                 pair<uint64_t, int> (10000, 1)}; // 1 10-ms
  Counter::MetricSet metric_set(1, "count");
  unique_ptr<Counter> counter(new Counter(test_time_config, metric_set));

  // sequence: 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
  metrics::tCountedEvent event1;
  counter->ProcessEvent(event1);
  std::chrono::milliseconds dur1(4);
  std::this_thread::sleep_for(dur1);
  metrics::tCountedEvent event2;
  counter->ProcessEvent(event2, 2);
  std::chrono::milliseconds dur2(11);
  std::this_thread::sleep_for(dur2);
  metrics::tCountedEvent event3;
  counter->ProcessEvent(event3);

  const vector<Counter::BinSet>& binsets = counter->GetBinsets();

  EXPECT_EQ(2, binsets.size());

  EXPECT_EQ(0, binsets.at(0).last_index);
  EXPECT_EQ(0, binsets.at(1).last_index);

  EXPECT_EQ(1, static_cast<int>(*binsets.at(0).bins.at(0).find("count")->second));
  EXPECT_EQ(0, static_cast<int>(*binsets.at(1).bins.at(0).find("count")->second));
}

TEST(ProcessEvent, MinIntegration) {
  Counter::CounterTimeConfig test_time_config = {pair<uint64_t, int> (2000, 1), // 1 2-ms
                                                 pair<uint64_t, int> (3000, 1)}; // 1 3-ms
  Counter::MetricSet metric_set(1, "min");
  unique_ptr<Counter> counter(new Counter(test_time_config, metric_set));

  // sequence: -20, 0, 0, 0, 0.5, 0, 5
  metrics::tCountedEvent event1;
  event1.value = -20;
  counter->ProcessEvent(event1);
  std::chrono::milliseconds dur1(4);
  std::this_thread::sleep_for(dur1);
  metrics::tCountedEvent event2;
  event2.value = 0.5;
  counter->ProcessEvent(event2, 2);
  std::chrono::milliseconds dur2(2);
  std::this_thread::sleep_for(dur2);
  metrics::tCountedEvent event3;
  event3.value = 5;
  counter->ProcessEvent(event3);

  Counter::MetricsMap metrics1 = counter->GetMetricsForInterval(2000);
  Counter::MetricsMap metrics2 = counter->GetMetricsForInterval(4000);
  Counter::MetricsMap metrics3 = counter->GetMetricsForInterval(11000);

  EXPECT_EQ(5, *metrics1.find("min")->second);
  EXPECT_EQ(0.5, *metrics2.find("min")->second);
  EXPECT_EQ(0.5, *metrics3.find("min")->second);
}

TEST(ProcessEvent, MaxIntegration) {
  Counter::CounterTimeConfig test_time_config = {pair<uint64_t, int> (2000, 1), // 1 2-ms
                                                 pair<uint64_t, int> (3000, 1)}; // 1 3-ms
  Counter::MetricSet metric_set(1, "max");
  unique_ptr<Counter> counter(new Counter(test_time_config, metric_set));

  // sequence: 20, 0, 0, 0, -0.5, 0, -5
  metrics::tCountedEvent event1;
  event1.value = 20;
  counter->ProcessEvent(event1);
  std::chrono::milliseconds dur1(4);
  std::this_thread::sleep_for(dur1);
  metrics::tCountedEvent event2;
  event2.value = -0.5;
  counter->ProcessEvent(event2, 2);
  std::chrono::milliseconds dur2(2);
  std::this_thread::sleep_for(dur2);
  metrics::tCountedEvent event3;
  event3.value = -5;
  counter->ProcessEvent(event3);

  Counter::MetricsMap metrics1 = counter->GetMetricsForInterval(2000);
  Counter::MetricsMap metrics2 = counter->GetMetricsForInterval(4000);
  Counter::MetricsMap metrics3 = counter->GetMetricsForInterval(11000);

  EXPECT_EQ(-5, *metrics1.find("max")->second);
  EXPECT_EQ(-0.5, *metrics2.find("max")->second);
  EXPECT_EQ(-0.5, *metrics3.find("max")->second);
}

}  // namespace test
}  // namespace counter
