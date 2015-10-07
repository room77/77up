// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the callback runner for structs.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_DESERIALIZATION_CALLBACK_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_DESERIALIZATION_CALLBACK_H_

#include "base/common.h"
#include "util/templates/sfinae.h"

namespace serial {

// Struct for setting default values based on type.
// If a function implements a 'bool DeserializationCallback()' function, it is called.
// If defined, the return value of function 'DeserializationCallback' is returned, otherwise true
// is returned.
struct DeserializationCallbackRunner {

  // Create all the checks.
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(DeserializationCallback);

  // ------------------------------------------------------------
  // Operator ()
  // ------------------------------------------------------------

  // For types that implement the DeserializationCallback function.
  template<class T>
  typename std::enable_if<
      has_member_func_sig_DeserializationCallback<T, bool()>::value, bool>::type
  operator()(T* v) {
    VLOG(5) << "callback: " << typeid(T).name();
    return v->DeserializationCallback();
  }

  template<class T>
  typename std::enable_if<
      !has_member_func_sig_DeserializationCallback<T, bool()>::value,
      bool>::type
  operator()(T* v) {
    VLOG(5) << "no callback: " << typeid(T).name();
    return true;
  }
};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_DESERIALIZATION_CALLBACK_H_
