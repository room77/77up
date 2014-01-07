// Copyright 2013 Room77, Inc.
// Author: Uygar Oztekin

// This is an example library where you can register context builders.
// We can use a single library that defines and registers multiple derived
// classes like this, or we can create multiple libraries, each defining and
// registering one context builder for more complex implementations.
// In order to use the registered context builders, the library that registers
// them needs to be added to the dependency.

#include "util/network/method/context_builder.h"

namespace network {

class ExampleContextBuilder : public ContextBuilder {
 public:
  string operator()(const string& input) const {
    return "Hello " + input;
  }
};

namespace {
  auto register_example_context_builder = ContextBuilder::bind("example",
      []{ return new ExampleContextBuilder; } );
}

}

