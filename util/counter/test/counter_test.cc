// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include <memory>
#include <utility>

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/counter.h"
#include "util/counter/counter_base.h"
#include "util/counter/metrics/metric_mock/metric_mock.h"

namespace counter {
namespace test {

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

// TODO(otasevic): use this class to setup the test cases
// TODO(otasevic): add more tests
class CounterTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {}

  // Tears down the test fixture.
  virtual void TearDown() {}

  // Sets up default values for all values.
  virtual void SetUpDefaults() {}
};

// Tests basic initialization and makes sure that the proper metrics are updated with an insert.
TEST_F(CounterTest, InitializationSanity) {
  Counter::CounterTimeConfig test_time_config = {pair<uint64_t, int> (5000, 2), // 2 5-ms
                                                 pair<uint64_t, int> (10000, 2)}; // 2 10-ms

  Counter::MetricSet test_metric_set {"mock_type_1",
                                      "mock_type_2"};
  unique_ptr<Counter> counter(new Counter(test_time_config, test_metric_set));

  const vector<Counter::BinSet>& binsets = counter->GetBinsets();

  metrics::test::MockMetricInterface* mock_metric_1_ =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).bins.at(0).find("mock_type_1")->second.get());
  metrics::test::MockMetricInterface* mock_total_metric_1_ =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).total_metrics.find("mock_type_1")->second.get());

  metrics::test::MockMetricInterface* mock_metric_2_ =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).bins.at(0).find("mock_type_2")->second.get());
  metrics::test::MockMetricInterface* mock_total_metric_2_ =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).total_metrics.find("mock_type_2")->second.get());

  ASSERT_NOTNULL(mock_metric_1_);
  ASSERT_NOTNULL(mock_total_metric_1_);
  ASSERT_NOTNULL(mock_metric_2_);
  ASSERT_NOTNULL(mock_total_metric_2_);

  metrics::tCountedEvent event1;

  EXPECT_CALL((*mock_metric_1_), Add(_)).Times(1);
  EXPECT_CALL((*mock_total_metric_1_), Add(_)).Times(1);
  EXPECT_CALL((*mock_metric_2_), Add(_)).Times(1);
  EXPECT_CALL((*mock_total_metric_2_), Add(_)).Times(1);

  counter->ProcessEvent(event1);
}

TEST_F(CounterTest, InsertIntoMultipleBins) {
  Counter::CounterTimeConfig test_time_config = {pair<uint64_t, int> (5000, 2), // 2 5-ms
                                                 pair<uint64_t, int> (10000, 2)}; // 2 10-ms
  Counter::MetricSet test_metric_set {"mock_type_1"};
  unique_ptr<Counter> counter(new Counter(test_time_config, test_metric_set));


  const vector<Counter::BinSet>& binsets = counter->GetBinsets();

  metrics::test::MockMetricInterface* mock_metric_bin_1 =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).bins.at(0).find("mock_type_1")->second.get());
  metrics::test::MockMetricInterface* mock_metric_bin_2 =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).bins.at(1).find("mock_type_1")->second.get());
  metrics::test::MockMetricInterface* mock_total_metric_1 =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).total_metrics.find("mock_type_1")->second.get());

  ASSERT_NOTNULL(mock_metric_bin_1);
  ASSERT_NOTNULL(mock_metric_bin_2);
  ASSERT_NOTNULL(mock_total_metric_1);

  EXPECT_CALL((*mock_metric_bin_1), Add(_)).Times(1);
  EXPECT_CALL((*mock_metric_bin_2), Add(_)).Times(1);
  EXPECT_CALL((*mock_total_metric_1), Add(_)).Times(2);

  // sequence: 1, 0, 0, 0, 0, 0, 1
  metrics::tCountedEvent event1;
  counter->ProcessEvent(event1);
  std::chrono::milliseconds dur(6);
  std::this_thread::sleep_for(dur);
  metrics::tCountedEvent event2;
  counter->ProcessEvent(event2);
}

// TODO(otasevic): complete this test
/*TEST_F(CounterTest, DroppingEvents) {
  Counter::CounterTimeConfig test_time_config = {pair<uint64_t, int> (3000, 1), // 1 5-ms
                                                 pair<uint64_t, int> (4000, 1)}; // 1 10-ms
  Counter::MetricSet test_metric_set {metrics::tMetrics::mock_type_1};
  unique_ptr<Counter> counter (new Counter(test_time_config, test_metric_set));


  const vector<Counter::BinSet>& binsets = counter->GetBinsets();

  metrics::test::MockMetricInterface* mock_metric_bin_1 =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).bins.at(0).find(metrics::tMetrics::mock_type_1)->second.get());
  metrics::test::MockMetricInterface* mock_metric_bin_2 =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(1).bins.at(0).find(metrics::tMetrics::mock_type_1)->second.get());
  metrics::test::MockMetricInterface* mock_total_metric_1 =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(0).total_metrics.find(metrics::tMetrics::mock_type_1)->second.get());
  metrics::test::MockMetricInterface* mock_total_metric_2 =
      dynamic_cast<metrics::test::MockMetricInterface*>(
          binsets.at(1).total_metrics.find(metrics::tMetrics::mock_type_1)->second.get());

  ASSERT_NOTNULL(mock_metric_bin_1);
  ASSERT_NOTNULL(mock_metric_bin_2);
  ASSERT_NOTNULL(mock_total_metric_1);
  ASSERT_NOTNULL(mock_total_metric_2);

  EXPECT_CALL((*mock_metric_bin_1), Add(_)).Times(2);
  //EXPECT_CALL((*mock_metric_bin_2), Add(_)).Times(2);
  //EXPECT_CALL((*mock_total_metric_1), Add(_)).Times(2);
  //EXPECT_CALL((*mock_total_metric_2), Add(_)).Times(0);

  // sequence: 1, 0, 0, 0, 0, 0, 0, 0, 0, 1
  metrics::tCountedEvent event1;
  counter->ProcessEvent(event1);
  std::chrono::milliseconds dur(10);
  std::this_thread::sleep_for(dur);
  metrics::tCountedEvent event2;
  counter->ProcessEvent(event2);
}*/

}  // namespace test
}  // namespace counter
