// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "arg_info.h"

namespace args {

// Specialization for string.
template<>
string ArgInfoWithData<string>::GetValueUnlocked() const {
  return *variable_;
}

template<>
string ArgInfoWithData<string>::ParseFromStringUnlocked(const string& value) {
  *variable_ = value;
  return "";
}

// Specialization for boolean.
template<>
string ArgInfoWithData<bool>::GetValueUnlocked() const {
  return *variable_ ? "true" : "false";
}

template<>
string ArgInfoWithData<bool>::ParseFromStringUnlocked(const string& value) {
  // Note: Empty string is a special case when the command line simply sets the
  // flag i.e. --set_flag_to_true rather than --set_flag_to_true=true.
  *variable_ = (value == "true" || value == "1" || value.empty()) ? true : false;
  return "";
}

}  // namespace args
