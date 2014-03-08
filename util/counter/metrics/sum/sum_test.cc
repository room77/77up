// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include <memory>
#include <utility>

#include "sum.h"

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/counter.h"

namespace counter {
namespace metrics {
namespace test {

// TODO(otasevic): add a stress test
TEST(Update, Sanity) {
  MetricInterface::unique_proxy sum1 = MetricInterface::make_unique("sum");
  tCountedEvent event;
  event.value = 1.5;
  sum1->Update(event, 2);

  EXPECT_EQ(3.0f, *sum1);
}

TEST(Plus, Sanity) {
  MetricInterface::unique_proxy sum1 = MetricInterface::make_unique("sum");
  tCountedEvent event1;
  event1.value = 1.5;
  sum1->Update(event1, 2);
  MetricInterface::unique_proxy sum2 = MetricInterface::make_unique("sum");
  tCountedEvent event2;
  event2.value = 2;
  sum2->Update(event2, 1);
  sum2->operator += (*sum1);

  EXPECT_EQ(5.0f, *sum2);
}

TEST(Minus, Sanity) {
  MetricInterface::unique_proxy sum1 = MetricInterface::make_unique("sum");
  tCountedEvent event1;
  event1.value = 1.5;
  sum1->Update(event1, 2);
  MetricInterface::unique_proxy sum2 = MetricInterface::make_unique("sum");
  tCountedEvent event2;
  event2.value = 2;
  sum2->Update(event2, 1);
  sum1->operator -= (*sum2);

  EXPECT_EQ(1, *sum1);
}

TEST(Minus, CannotBeNegative) {
  MetricInterface::unique_proxy sum1 = MetricInterface::make_unique("sum");
  tCountedEvent event1;
  event1.value = 1.5;
  sum1->Update(event1, 2);
  MetricInterface::unique_proxy sum2 = MetricInterface::make_unique("sum");
  tCountedEvent event2;
  event2.value = 2;
  sum2->Update(event2, 1);
  sum2->operator -= (*sum1);

  EXPECT_EQ(0, *sum2);
}

TEST(Reset, Sanity) {
  MetricInterface::unique_proxy sum1 = MetricInterface::make_unique("sum");
  tCountedEvent event;
  sum1->Update(event, 2);
  sum1->Reset();

  EXPECT_EQ(0, *sum1);
}

}  // namespace test
}  // namespace metrics
}  // namespace counter
