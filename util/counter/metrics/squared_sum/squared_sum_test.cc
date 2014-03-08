// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include <memory>
#include <utility>

#include "squared_sum.h"

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/counter.h"

namespace counter {
namespace metrics {
namespace test {

// TODO(otasevic): add a stress test
TEST(Update, Sanity) {
  MetricInterface::unique_proxy squared_sum1 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event;
  event.value = 1.5;
  squared_sum1->Update(event, 2);

  EXPECT_EQ(4.5f, *squared_sum1);
}

TEST(Plus, Sanity) {
  MetricInterface::unique_proxy squared_sum1 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event1;
  event1.value = 1.5;
  squared_sum1->Update(event1, 2);
  MetricInterface::unique_proxy squared_sum2 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event2;
  event2.value = 2;
  squared_sum2->Update(event2, 1);
  squared_sum2->operator += (*squared_sum1);

  EXPECT_EQ(8.5, *squared_sum2);
}

TEST(Minus, Sanity) {
  MetricInterface::unique_proxy squared_sum1 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event1;
  event1.value = 1.5;
  squared_sum1->Update(event1, 2);
  MetricInterface::unique_proxy squared_sum2 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event2;
  event2.value = 2;
  squared_sum2->Update(event2, 1);
  squared_sum1->operator -= (*squared_sum2);

  EXPECT_EQ(0.5, *squared_sum1);
}

TEST(Minus, CannotBeNegative) {
  MetricInterface::unique_proxy squared_sum1 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event1;
  event1.value = 1.5;
  squared_sum1->Update(event1, 2);
  MetricInterface::unique_proxy squared_sum2 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event2;
  event2.value = 2;
  squared_sum2->Update(event2, 1);
  squared_sum2->operator -= (*squared_sum1);

  EXPECT_EQ(0, *squared_sum2);
}

TEST(Reset, Sanity) {
  MetricInterface::unique_proxy squared_sum1 = MetricInterface::make_unique("squared_sum");
  tCountedEvent event;
  squared_sum1->Update(event, 2);
  squared_sum1->Reset();

  EXPECT_EQ(0, *squared_sum1);
}

}  // namespace test
}  // namespace metrics
}  // namespace counter
