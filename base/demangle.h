// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_BASE_DEMANGLE_H_
#define _PUBLIC_BASE_DEMANGLE_H_

#include <string>
#include <typeinfo>

namespace base {

// Returns the unmangled name given the type info.
// e.g. 3fooISsLi10EE => foo<std::string, 10>
std::string PrettyNameFromTypeInfo(const std::string& type_info_name);

// Returns the unmangled name given the type.
// e.g. foo<std::string, 10> x;
// DemangleType(x) => foo<std::string, 10>
template<typename T>
std::string PrettyNameFromType(const T& obj = T()) {
  static std::string type_pretty_name =
      PrettyNameFromTypeInfo(typeid(T).name());
  return type_pretty_name;
}

}  // namespace base


#endif  // _PUBLIC_BASE_DEMANGLE_H_
