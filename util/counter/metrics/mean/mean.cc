// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/counter/metrics/mean/mean.h"

namespace counter {
namespace metrics {

// register
auto reg_mean = MetricInterface::bind("mean", [] { return new Mean(); });

}  // namespace metrics
}  // namespace counter
