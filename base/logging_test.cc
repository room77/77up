// Copyright 2007 OpTrip, Inc.

#include "base/logging.h"

int main() {
  const char *s = "hello";
  LOG(INFO) << s;
  LOG(INFO) << s;

  // ASSERT(0) << "Assert failed: " << 1;

  int val = 0;
  ASSERT_DEV(true) << " Failed: " << val;
  // Use val if not in dev env to avoid compiler unused-variable warning.
  cout << "val = " << val << endl;

  for (int i = 0; i < 100; ++i) {
    LOG_EVERY_N(INFO, 10) << "This should log every 10 calls. This one is: " << i;
  }
  return 0;
}
