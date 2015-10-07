// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/metrics/metric_mock/metric_mock.h"

namespace counter {
namespace metrics {
namespace test {

// register
auto reg_mock1 = MetricInterface::bind("mock_type_1", [] { return new MockMetricInterface(); });

auto reg_mock2 = MetricInterface::bind("mock_type_2", [] { return new MockMetricInterface(); });

}  // namespace test
}  // namespace metrics
}  // namespace counter
