// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/metrics/count/count.h"

namespace counter {
namespace metrics {

// register
auto reg_count = MetricInterface::bind("count", [] { return new Count(); });

}  // namespace metrics
}  // namespace counter
