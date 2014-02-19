// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/metrics/sum/sum.h"

namespace counter {
namespace metrics {

// register
auto reg_sum = MetricInterface::bind("sum", [] { return new Sum(); });

}  // namespace metrics
}  // namespace counter
