// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/type_handlers/deserialization_callback.h"

#include "util/serial/type_handlers/test_util.h"
#include "test/cc/test_main.h"

namespace serial {
namespace test {

TEST(DesrializationCallbackTest, Sanity) {
  { TestDataWithSerialization<TestDataWithZeroDefaults> a;
    DeserializationCallbackRunner()(&a);
  }

  { TestDataCallback<TestDataWithZeroDefaults> a;
    DeserializationCallbackRunner()(&a);
    EXPECT_TRUE(a.callback_);
  }
}

}  // namespace test
}  // namespace serial
