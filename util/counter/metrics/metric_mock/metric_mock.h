// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_UTIL_COUNTER_METRICS_METRIC_MOCK_METRIC_MOCK_H_
#define _PUBLIC_UTIL_COUNTER_METRICS_METRIC_MOCK_METRIC_MOCK_H_

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/metrics/metric.h"

namespace counter {
namespace metrics {
namespace test {

using ::testing::_;
using ::testing::ReturnRef;

class MockMetricInterface : public MetricInterface {
 public:
  virtual ~MockMetricInterface() {}

  MockMetricInterface() {
    ON_CALL(*this, Add(_))
        .WillByDefault(ReturnRef(*this));
    ON_CALL(*this, Subtract(_))
        .WillByDefault(ReturnRef(*this));
  }

  MOCK_METHOD1(Add, MockMetricInterface&(const MetricInterface& metric));
  virtual MockMetricInterface& operator+=(const MetricInterface& c) {
    return Add(c);
  }

  MOCK_METHOD1(Subtract, MockMetricInterface&(const MetricInterface& metric));
  virtual MockMetricInterface& operator-=(const MetricInterface& c) {
    return Subtract(c);
  }

  MOCK_CONST_METHOD0(Get, float());
  virtual operator float() const { return Get(); }

  MOCK_METHOD2(Update, void(const tCountedEvent& event, int number_of_events));

  MOCK_METHOD0(Reset, void());
  MOCK_CONST_METHOD0(GetNumElements, int());
};

}  // namespace test
}  // namespace metrics
}  // namespace counter


#endif  // _PUBLIC_UTIL_COUNTER_METRICS_METRIC_MOCK_METRIC_MOCK_H_
