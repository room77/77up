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
//
TEST(Update, Sanity) {
  MetricInterface::unique_proxy count1 = MetricInterface::make_unique("count");
  tCountedEvent event;
  count1->Update(event, 2);

  EXPECT_EQ(2, *count1);
}

TEST(Update, WithNegativeNumEvents) {
  MetricInterface::unique_proxy count1 = MetricInterface::make_unique("count");
  tCountedEvent event;
  count1->Update(event, -2);

  EXPECT_EQ(0, *count1);
}

TEST(Plus, Sanity) {
  MetricInterface::unique_proxy count1 = MetricInterface::make_unique("count");
  tCountedEvent event;
  count1->Update(event, 2);
  MetricInterface::unique_proxy count2 = MetricInterface::make_unique("count");
  count2->operator += (*count1);

  EXPECT_EQ(2, *count2);
}

TEST(Minus, Sanity) {
  MetricInterface::unique_proxy count1 = MetricInterface::make_unique("count");
  tCountedEvent event;
  count1->Update(event, 2);
  MetricInterface::unique_proxy count2 = MetricInterface::make_unique("count");
  count2->Update(event, 1);
  count1->operator -= (*count2);

  EXPECT_EQ(1, *count1);
}

TEST(Minus, NonNegativeCount) {
  MetricInterface::unique_proxy count1 = MetricInterface::make_unique("count");
  tCountedEvent event;
  count1->Update(event, 2);
  MetricInterface::unique_proxy count2 = MetricInterface::make_unique("count");
  count2->Update(event, 1);
  count2->operator -= (*count1);

  EXPECT_EQ(0, *count2);
}

TEST(Reset, Sanity) {
  MetricInterface::unique_proxy count1 = MetricInterface::make_unique("count");
  tCountedEvent event;
  count1->Update(event, 2);
  count1->Reset();

  EXPECT_EQ(0, *count1);
}

}  // namespace test
}  // namespace metrics
}  // namespace counter
