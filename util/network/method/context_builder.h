// Copyright 2013 Room77, Inc.
// Author: Uygar Oztekin

// A class hierarchy to build various context types. We will probably register
// a few of them to cover most cases we care about.

// Contexts may contain high level information such as language, country,
// desired currency etc.

#ifndef _PUBLIC_UTIL_NETWORK_METHOD_CONTEXT_BUILDER_H_
#define _PUBLIC_UTIL_NETWORK_METHOD_CONTEXT_BUILDER_H_

#include "base/common.h"
#include "util/factory/factory.h"

namespace network {

class ContextBuilder : public Factory<ContextBuilder> {
 public:
  virtual ~ContextBuilder() {}

  // Define the api here. Inputs / output types to be determined. I am using
  // string for now for illustration purposes, but they probably end up being
  // structs.
  virtual string operator()(const string& input) const = 0;
};

}

#endif  // _PUBLIC_UTIL_NETWORK_METHOD_CONTEXT_BUILDER_H_
