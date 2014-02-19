// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/metrics/min/min.h"

namespace counter {
namespace metrics {

// register
auto reg_min = MetricInterface::bind("min", [] { return new Min(); });

}  // namespace metrics
}  // namespace counter
