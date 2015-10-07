// Copyright 2013 Room77, Inc.
// Author: B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_CONVERT_DEFAULT_CONVERTER_H_
#define _PUBLIC_UTIL_STORE_CONVERT_DEFAULT_CONVERTER_H_

#include "util/serial/serializer.h"

namespace store {
namespace convert {

// Default converter definition.
template<class UserKey, class StoreKey>
struct DefaultConverter {
  static constexpr bool is_ordered = false;
  StoreKey operator()(const UserKey&) const;
  UserKey operator()(const StoreKey&) const;
};

// Specialization when the types are the same.
template<class T>
struct DefaultConverter<T, T> {
  static constexpr bool is_ordered = true;
  const T& operator()(const T& t) const { return t; }
};

// If store key / data is string, use binary serialization.
template<class T>
struct DefaultConverter<T, string> {
  static constexpr bool is_ordered  = false;
  string operator()(const T& t) const { return serial::Serializer::ToBinary(t); }
  T operator()(const string& s) const {
    T t;
    ASSERT(serial::Serializer::FromBinary<T>(s, &t));
    return t;
  }
};

// Specialization when the types are both strings (needed to disambiguate
// between DefaultConverter<T, T> and DefaultConverter<T, string>).
template<>
struct DefaultConverter<string, string> {
  static constexpr bool is_ordered = true;
  const string& operator()(const string& t) const { return t; }
};

}
}

#endif  // _PUBLIC_UTIL_STORE_CONVERT_DEFAULT_CONVERTER_H_
