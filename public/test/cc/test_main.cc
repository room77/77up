// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)
//
// The default main file that can be used to run unit tests.

#include "util/init/main.h"
#include "test/cc/test_main.h"

extern string gFlag_init_include;

int main(int argc, char** argv) {
  // Init the google mock framework.
  // This in turn inits the google test framework.
  testing::InitGoogleMock(&argc, argv);

  // Only include test inits.
  gFlag_init_include = "test";

  // Init R77.
  r77::R77Init(argc, argv);

  int status = RUN_ALL_TESTS();

  // Shutdown R77.
  r77::R77Shutdown();

  return status;
}
