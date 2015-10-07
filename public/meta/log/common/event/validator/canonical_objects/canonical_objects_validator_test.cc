// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/event/validator/canonical_objects/canonical_objects_validator.h"

#include "test/cc/test_main.h"

namespace logging {
namespace event {
namespace test {

// Test that CheckRating function works properly
TEST(CheckRating, Sanity) {
  EXPECT_TRUE(CheckRating(3.5));
  EXPECT_TRUE(CheckRating(0));
  EXPECT_FALSE(CheckRating(3.51));
  EXPECT_FALSE(CheckRating(5.5));
  EXPECT_TRUE(CheckRating(5));
  EXPECT_FALSE(CheckRating(-1));
}

TEST(CheckNumNights, Sanity) {
  EXPECT_TRUE(CheckNumNights(true, 0));
  EXPECT_TRUE(CheckNumNights(false, 1));
  EXPECT_FALSE(CheckNumNights(true, 5));
  EXPECT_FALSE(CheckNumNights(false, 0));
}

}  // namespace test
}  // namespace event
}  // namespace logging

