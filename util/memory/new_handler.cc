// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

#include "base/common.h"

// intercept memory allocation failures and force a stack trace
struct InitNewHandler {
  static void NewHandler () {
    cout << "\n*** Memory allocation Failed! About to segfault ***" << endl;
    // Cause a segfault.
    void* ptr = nullptr; // Create a var to get around enabled compiler warning.
    *static_cast<int*>(ptr) = 0;
  }
  InitNewHandler() { set_new_handler(InitNewHandler::NewHandler); }
} auto_init_new_handler;
