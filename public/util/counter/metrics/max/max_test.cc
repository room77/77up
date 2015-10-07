// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include <memory>
#include <utility>

#include "max.h"

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/counter.h"

namespace counter {
namespace metrics {
namespace test {

// TODO(otasevic): add a stress test
TEST(Update, Sanity) {
  MetricInterface::unique_proxy max1 = MetricInterface::make_unique("max");
  tCountedEvent event;
  event.value = 1.5;
  max1->Update(event, 1);

  EXPECT_EQ(1.5f, *max1);
}

TEST(Plus, Sanity) {
  MetricInterface::unique_proxy max1 = MetricInterface::make_unique("max");
  tCountedEvent event1;
  event1.value = 1.5;
  max1->Update(event1, 2);
  MetricInterface::unique_proxy max2 = MetricInterface::make_unique("max");
  tCountedEvent event2;
  event2.value = 1;
  max2->Update(event2, 1);
  max2->operator += (*max1);

  EXPECT_EQ(1.5f, *max2);
}

TEST(Reset, Sanity) {
  MetricInterface::unique_proxy max1 = MetricInterface::make_unique("max");
  tCountedEvent event;
  max1->Update(event, 2);
  max1->Reset();

  EXPECT_EQ(numeric_limits<int>::min(), *max1);
}

}  // namespace test
}  // namespace metrics
}  // namespace counter
