// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/metrics/max/max.h"

namespace counter {
namespace metrics {

// register
auto reg_max = MetricInterface::bind("max", [] { return new Max(); });

}  // namespace metrics
}  // namespace counter
