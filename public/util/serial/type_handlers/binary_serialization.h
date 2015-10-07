// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the Binary serialization for each type.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_BINARY_SERIALIZATION_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_BINARY_SERIALIZATION_H_

#include <memory>
#include <ostream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "base/common.h"
#include "util/serial/encoding/endian.h"
#include "util/serial/utils/serializer_util.h"
#include "util/serial/types/varint.h"
#include "util/templates/sfinae.h"

namespace serial {

// Struct for setting default values based on type.
struct BinarySerializationByType {
  // Create all the checks.
  CREATE_MEMBER_FUNC_CHECK(get);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(size);
  CREATE_MEMBER_TYPE_CHECK(const_iterator);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(ToBinary);

  // Default implementation for specializations.
  // By default there is no specialization.
  template<typename T, typename = std::true_type>
  struct specializations : std::false_type {};

  // All specializations. Any specializations should be added here to avoid
  // ambiguous function instantitaion for different types.
  template<typename T>
  struct specializations<T, std::integral_constant<bool,
      std::is_pointer<T>::value ||
      (has_member_func_sig_ToBinary<T,
          void(ostream&, const BinarySerializationParams&)>::value) ||
      has_member_func_sig_ToBinary<T, void(ostream&)>::value ||
      has_member_func_get<T>::value ||
      (has_member_func_sig_size<T, size_t ()>::value &&
          has_member_type_const_iterator<T>::value)>> : std::true_type {};

  // All types that can be set to 0.
  template<typename T>
  struct is_serializable : std::integral_constant<bool,
      (std::is_arithmetic<T>::value || std::is_enum<T>::value)
      && !specializations<T>::value> {};

  // ------------------------------------------------------------
  // Operator ()
  // ------------------------------------------------------------

  // For all integer types.
  template<typename T>
  typename std::enable_if<(is_integral<T>::value  || std::is_enum<T>::value),
      BinarySerializationByType&>::type
  operator()(ostream& out, const T v) {
    VLOG(5) << "basic: " << typeid(T).name() << ", size: " << sizeof(T);
    // Serialize everything as varint.
    varint<T> temp(v);
    operator()(out, temp);
    return *this;
  }

  // For all floating types.
  template<typename T>
  typename std::enable_if<is_floating_point <T>::value,
      BinarySerializationByType&>::type
  operator()(ostream& out, const T v) {
    VLOG(5) << "basic: " << typeid(T).name() << ", size: " << sizeof(T);
    T temp = v;
    // Always serialize in little endian format.
    endian::HToLE(&temp);
    out.write(reinterpret_cast<char*>(&temp), sizeof(T));
    return *this;
  }

  // For all pointer types.
  template<typename T>
  typename std::enable_if<std::is_pointer<T>::value,
      BinarySerializationByType&>::type
  operator()(ostream& out, const T v) {
    VLOG(5) << "pointer: " << typeid(T).name();
    if (v != nullptr) {
      operator()(out, *v);
    } else {
      typename std::remove_pointer<T>::type def =
          typename std::remove_pointer<T>::type();
      operator()(out, def);
    }
    return *this;
  }

  // For all types that are iterable.
  template<typename T>
  typename std::enable_if<has_member_func_sig_size<T, size_t ()>::value &&
      has_member_type_const_iterator<T>::value, BinarySerializationByType&>::type
  operator()(ostream& out, const T& v) {
    // write out the container's size, then its content
    VLOG(5) << "container:" << typeid(T).name() << ", size: " << v.size();

    // Write the size of the container.
    operator()(out, v.size());

    // Dump each element.
    for (const auto& elem : v) operator()(out, elem);

    return *this;
  }

  // For all types that have a 'get' function.
  template<typename T>
  typename std::enable_if<has_member_func_get<T>::value,
      BinarySerializationByType&>::type
  operator()(ostream& out, const T& v) {
    VLOG(5) << "Gettable:" << typeid(T).name();
    operator()(out, v.get());
    return *this;
  }

  // For all nested structs.
  template<typename T>
  typename std::enable_if<
      has_member_func_sig_ToBinary<T, void(ostream&, const BinarySerializationParams&)>::value,
      BinarySerializationByType&>::type
  operator()(ostream& out, const T& v) {
    VLOG(5) << "struct: " << typeid(T).name();
    v.ToBinary(out, params);
    return *this;
  }

  // For all custom types that implement ToBinary for serialization.
  template<typename T>
  typename std::enable_if<
      !has_member_func_sig_ToBinary<T, void(ostream&, const BinarySerializationParams&)>::value &&
      has_member_func_sig_ToBinary<T, void(ostream&)>::value,
      BinarySerializationByType&>::type
  operator()(ostream& out, const T& v) {
    VLOG(5) << "struct: " << typeid(T).name();
    v.ToBinary(out);
    return *this;
  }

  // For pairs.
  template<typename T1, typename T2>
  BinarySerializationByType& operator()(ostream& out, const pair<T1, T2>& v) {
    VLOG(5) << "pair:<" << typeid(T1).name() << ", " << typeid(T2).name() << ">";

    operator()(out, v.first);
    operator()(out, v.second);
    return *this;
  }

  // For Strings.
  // We specialize strings for optimization.
  template<typename T>
  BinarySerializationByType& operator()(ostream& out,
                                        const basic_string<T>& v) {
    VLOG(5) << "string:" << typeid(T).name() << ", size: " << v.size();

    // Write the size of string.
    operator()(out, v.size());
    out.write(v.c_str(), v.size());
    return *this;
  }

  // For one dimension arrays.
  // Note: We currently do not support multi-dimension array.
  template<typename T, std::size_t N>
  BinarySerializationByType& operator()(ostream& out, const T(&v)[N]) {
    VLOG(5) << "Arr:" << typeid(T).name();
    operator()(out, N);
    for (size_t i = 0; i < N; ++i) operator()(out, v[i]);
    return *this;
  }

  // ------------------------------------------------------------
  // Constructor
  // ------------------------------------------------------------
  BinarySerializationByType(const BinarySerializationParams& params =
      BinarySerializationParams()) : params(params) {}

  BinarySerializationParams params;
};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_BINARY_SERIALIZATION_H_
