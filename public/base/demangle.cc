// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "base/demangle.h"

#include <cxxabi.h>
#include <stdlib.h>
#include <iostream>

namespace base {

// Returns the unmangled name given the type info.
// e.g. 3fooISsLi10EE => foo<std::string, 10>
std::string PrettyNameFromTypeInfo(const std::string& type_info_name) {
  std::size_t length = 0;
  int status = 0;
  char* unmangled_name = abi::__cxa_demangle(type_info_name.c_str(), nullptr,
                                             &length, &status);

  if (status || !length || unmangled_name == nullptr) return type_info_name;

  std::string res(unmangled_name);
  free(unmangled_name);  // We need to free the unmangled name.
  return res;
}

}  // namespace base

