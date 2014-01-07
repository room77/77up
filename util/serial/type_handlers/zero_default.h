// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the zero default values for each type.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_ZERO_DEFAULT_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_ZERO_DEFAULT_H_

#include <memory>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>


#include "base/common.h"
#include "util/templates/sfinae.h"

namespace serial {

// Struct for setting default values based on type.
struct DefaultZeroByType {

  // Create all the checks.
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(reset);
  CREATE_MEMBER_CHECK(element_type);

  // Default implementation for specializations.
  // By default there is no specialization.
  template<typename T, typename = std::true_type>
  struct specializations : std::false_type {};

  // All specializations. Any specializations should be added here to avoid
  // ambiguous function instantitaion for different types.
  template<typename T>
  struct specializations<T, std::integral_constant<bool,
      (has_member_func_sig_reset<T, void()>::value &&
          has_member_element_type<T>::value) ||
      std::is_array<T>::value>> : std::true_type {};

  // ------------------------------------------------------------
  // Operator ()
  // ------------------------------------------------------------

  // Default: For all basic types.
  template<class T>
  typename std::enable_if<!specializations<T>::value,
      DefaultZeroByType&>::type
  operator()(T& v) {
    VLOG(5) << "default: " << typeid(T).name();
    v = T();
    return *this;
  }

  // For all types that have a 'reset' function and have 'element_type'.
  template<typename T>
  typename std::enable_if<has_member_func_sig_reset<T, void()>::value &&
      has_member_element_type<T>::value, DefaultZeroByType&>::type
  operator()(T& v) {
    VLOG(5) << "ptr:" << typeid(T).name();
    v.reset(new typename T::element_type());
    operator()(*v);
    return *this;
  }

  // For one dimension arrays.
  // Note: We currently do not support multi-dimension array.
  template<typename T>
  typename std::enable_if<std::is_array<T>::value, DefaultZeroByType&>::type
  operator()(T& v) {
    VLOG(5) << "Arr:" << typeid(T).name();
    for (size_t i = 0; i < std::extent<T>::value; ++i)
      operator()(v[i]);
    return *this;
  }
};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_ZERO_DEFAULT_H_
