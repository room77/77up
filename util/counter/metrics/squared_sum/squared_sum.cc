// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/metrics/squared_sum/squared_sum.h"

namespace counter {
namespace metrics {

// register
auto reg_squared_sum = MetricInterface::bind("squared_sum", [] { return new SquaredSum(); });

}  // namespace metrics
}  // namespace counter
