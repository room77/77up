// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include <memory>
#include <utility>

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/counter.h"

namespace counter {
namespace metrics {
namespace test {

// TODO(otasevic): add a stress test
TEST(Update, Sanity) {
  MetricInterface::unique_proxy min1 = MetricInterface::make_unique("min");
  tCountedEvent event;
  event.value = 1.5;
  min1->Update(event, 1);

  EXPECT_EQ(1.5f, *min1);
}

TEST(Plus, Sanity) {
  MetricInterface::unique_proxy min1 = MetricInterface::make_unique("min");
  tCountedEvent event1;
  event1.value = 1.5;
  min1->Update(event1, 2);
  MetricInterface::unique_proxy min2 = MetricInterface::make_unique("min");
  tCountedEvent event2;
  event2.value = 1;
  min2->Update(event2, 1);
  min2->operator += (*min1);

  EXPECT_EQ(1.0f, *min2);
}

TEST(Reset, Sanity) {
  MetricInterface::unique_proxy min1 = MetricInterface::make_unique("min");
  tCountedEvent event;
  min1->Update(event, 2);
  min1->Reset();

  EXPECT_EQ(numeric_limits<float>::max(), *min1);
}

}  // namespace test
}  // namespace metrics
}  // namespace counter
