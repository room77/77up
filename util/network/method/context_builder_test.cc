// Copyright 2013 Room77, Inc.
// Author: B. Uygar Oztekin

// @include "context_builders.cc"

#include <iostream>
#include "base/common.h"
#include "util/network/method/context_builder.h"
#include "util/init/main.h"

int init_main() {
  // Note that example context builder is not know by this class, it is not even
  // declared in a header file.
  auto p = network::ContextBuilder::make_shared("example");
  cout << p->operator()("World") << endl;
  return 0;
}
