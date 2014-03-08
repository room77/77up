// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include <memory>
#include <utility>

#include "mean.h"

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/counter/counter.h"

namespace counter {
namespace metrics {
namespace test {

// TODO(otasevic): add a stress test
TEST(Update, Sanity) {
  MetricInterface::unique_proxy mean1 = MetricInterface::make_unique("mean");
  tCountedEvent event1;
  event1.value = 2.6;
  mean1->Update(event1, 1);
  tCountedEvent event2;
  event2.value = 2;
  mean1->Update(event2, 2);

  EXPECT_EQ(2.2f, *mean1);
}

TEST(Update, WithNegativeNumEvents) {
  MetricInterface::unique_proxy mean1 = MetricInterface::make_unique("mean");
  tCountedEvent event1;
  event1.value = 2.6;
  mean1->Update(event1, 1);
  tCountedEvent event2;
  event2.value = 2;
  mean1->Update(event2, -2);

  EXPECT_EQ(2.6f, *mean1);
}

TEST(Plus, Sanity) {
  MetricInterface::unique_proxy mean1 = MetricInterface::make_unique("mean");
  tCountedEvent event1;
  event1.value = 2.6;
  mean1->Update(event1, 1);
  MetricInterface::unique_proxy mean2 = MetricInterface::make_unique("mean");
  tCountedEvent event2;
  event2.value = 2;
  mean2->Update(event2, 2);
  mean2->operator += (*mean1);

  EXPECT_EQ(2.2f, *mean2);
}

TEST(Minus, Sanity) {
  MetricInterface::unique_proxy mean1 = MetricInterface::make_unique("mean");
  tCountedEvent event1;
  event1.value = 2.7;
  mean1->Update(event1, 1);
  MetricInterface::unique_proxy mean2 = MetricInterface::make_unique("mean");
  tCountedEvent event2;
  event2.value = 2;
  mean2->Update(event2, 2);
  mean2->operator -= (*mean1);

  EXPECT_EQ(1.3f, *mean2);
}

TEST(Minus, NonNegativeCount) {
  MetricInterface::unique_proxy mean1 = MetricInterface::make_unique("mean");
  tCountedEvent event1;
  event1.value = 2.7;
  mean1->Update(event1, 1);
  MetricInterface::unique_proxy mean2 = MetricInterface::make_unique("mean");
  tCountedEvent event2;
  event2.value = 2;
  mean2->Update(event2, 2);
  mean1->operator -= (*mean2);

  EXPECT_EQ(0, *mean1);
}

TEST(Minus, SubAllTheElements) {
  MetricInterface::unique_proxy mean1 = MetricInterface::make_unique("mean");
  tCountedEvent event1;
  event1.value = 2.7;
  mean1->Update(event1, 2);
  MetricInterface::unique_proxy mean2 = MetricInterface::make_unique("mean");
  tCountedEvent event2;
  event2.value = 2;
  mean2->Update(event2, 2);
  mean1->operator -= (*mean2);

  EXPECT_EQ(0, *mean1);
}

TEST(Reset, Sanity) {
  MetricInterface::unique_proxy count1 = MetricInterface::make_unique("mean");
  tCountedEvent event;
  count1->Update(event, 2);
  count1->Reset();

  EXPECT_EQ(0, static_cast<int>(*count1));
}

}  // namespace test
}  // namespace metrics
}  // namespace counter
