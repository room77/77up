// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the Binary deserialization for each type.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_BINARY_DESERIALIZATION_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_BINARY_DESERIALIZATION_H_

#include <istream>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "base/common.h"
#include "util/serial/encoding/endian.h"
#include "util/serial/utils/serializer_util.h"
#include "util/serial/utils/stream_util.h"
#include "util/serial/types/varint.h"
#include "util/templates/sfinae.h"

namespace serial {

// Struct for setting default values based on type.
struct BinaryDeSerializationByType {
  // Create all the checks.
  CREATE_MEMBER_FUNC_CHECK(clear);
  CREATE_MEMBER_TYPE_CHECK(value_type);
  CREATE_MEMBER_TYPE_CHECK(const_iterator);
  CREATE_MEMBER_FUNC_FIXED_SOFT_SIG_CHECK(typename T::iterator, insert,
                                          typename T::iterator,
                                          const typename T::value_type&);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(reserve);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(reset);
  CREATE_MEMBER_TYPE_CHECK(element_type);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(FromBinary);

  // Default implementation for specializations.
  // By default there is no specialization.
  template<typename T, typename = std::true_type>
  struct specializations : std::false_type {};

  // All specializations. Any specializations should be added here to avoid
  // ambiguous function instantitaion for different types.
  template<typename T>
  struct specializations<T, std::integral_constant<bool,
      std::is_pointer<T>::value ||
      (has_member_func_sig_FromBinary<T,
          bool (istream&, const BinaryDeSerializationParams&)>::value) ||
      has_member_func_sig_FromBinary<T, bool (istream&)>::value ||
      (has_member_func_clear<T>::value &&
          has_member_type_value_type<T>::value &&
          has_member_type_const_iterator<T>::value &&
          has_member_func_fixed_sig_insert<T>::value) ||
      (has_member_func_sig_reset<T, void()>::value &&
                has_member_type_element_type<T>::value)>> : std::true_type {};

  // All types that can be set to 0.
  template<typename T>
  struct is_serializable : std::integral_constant<bool,
      (std::is_arithmetic<T>::value || std::is_enum<T>::value)
      && !specializations<T>::value> {};

  // ------------------------------------------------------------
  // Operator ()
  // ------------------------------------------------------------

  // Default Implementation for unsupported types.
  template<typename T>
  typename std::enable_if<!is_serializable<T>::value &&
      !specializations<T>::value, BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "UnSupported: " << typeid(T).name();
    ASSERT(0) << "Unsupported type: " << typeid(T).name();
    return *this;
  }

  // For all integer types.
  template<typename T>
  typename std::enable_if<(is_integral<T>::value  || std::is_enum<T>::value),
      BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "basic: " << typeid(T).name() << ", size: " << sizeof(T);
    ASSERT_NOTNULL(v);
    varint<T> temp;
    operator()(in, &temp);

    if (!in.fail()) *v = temp;
    else util::LogParsingError(in, -1,
                               string("Could not parse basic type: ") + typeid(T).name(),
                               params.err);
    return *this;
  }

  // For all floating types.
  template<typename T>
  typename std::enable_if<is_floating_point<T>::value,
      BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "basic: " << typeid(T).name();
    ASSERT_NOTNULL(v);
    in.read(reinterpret_cast<char*>(v), sizeof(T));

    // Deserialize from Little endian to Host.
    if (!in.fail()) endian::LEToH(v);
    else util::LogParsingError(in, -1,
                               string("Could not parse ") + typeid(T).name(),
                               params.err);
    return *this;
  }

  // For all pointer types.
  template<typename T>
  typename std::enable_if<std::is_pointer<T>::value,
      BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "pointer: " << typeid(T).name();

    T new_val = new typename std::remove_pointer<T>::type();
    *v = new_val;
    operator()(in, new_val);

    return *this;
  }

  // For all types that are iterable and have reserve. This avoids multiple
  // reallocations.
  template<typename T>
  typename std::enable_if<has_member_func_clear<T>::value &&
      has_member_type_value_type<T>::value && has_member_type_const_iterator<T>::value &&
      has_member_func_fixed_sig_insert<T>::value &&
      has_member_func_sig_reserve<T, void (size_t)>::value,
      BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    // Read the container's size, then its content.
    VLOG(5) << "container with reserve:" << typeid(T).name();
    v->clear();

    // Get the size of the container.
    size_t size = 0;
    operator()(in, &size);
    if (in.fail()) return *this;
    v->reserve(size);
    for (size_t i = 0; i < size; ++i) {
      typename T::value_type value;
      operator()(in, &value);
      if (in.fail()) break;
      v->insert(v->end(), value);
    }
    return *this;
  }

  // For all types that are iterable but do not have reserve.
  template<typename T>
  typename std::enable_if<has_member_func_clear<T>::value &&
      has_member_type_value_type<T>::value && has_member_type_const_iterator<T>::value &&
      has_member_func_fixed_sig_insert<T>::value &&
      !has_member_func_sig_reserve<T, void (size_t)>::value,
      BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    // Read the container's size, then its content.
    VLOG(5) << "container without reserve:" << typeid(T).name();
    v->clear();
    // Get the size of the container.
    size_t size = 0;
    operator()(in, &size);
    if (in.fail()) return *this;

    for (size_t i = 0; i < size; ++i) {
      typename T::value_type value;
      operator()(in, &value);
      if (in.fail()) break;
      v->insert(v->end(), value);
    }
    return *this;
  }

  // For all types that have a 'reset' function and have 'element_type'.
  template<typename T>
  typename std::enable_if<has_member_func_sig_reset<T, void()>::value &&
      has_member_type_element_type<T>::value, BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "ptr:" << typeid(T).name();
    v->reset(new typename T::element_type());
    operator()(in, v->get());
    return *this;
  }

  // For all nested structs.
  template<typename T>
  typename std::enable_if<
      has_member_func_sig_FromBinary<T, bool (istream&, const BinaryDeSerializationParams&)>::value,
      BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "struct: " << typeid(T).name();
    if (!v->FromBinary(in, params)) in.setstate(istream::failbit);
    return *this;
  }

  // For all custom types that implement FromBinary for deserialization.
  template<typename T>
  typename std::enable_if<
      (!has_member_func_sig_FromBinary<T,
          bool (istream&, const BinaryDeSerializationParams&)>::value) &&
      has_member_func_sig_FromBinary<T, bool (istream&)>::value,
      BinaryDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "struct: " << typeid(T).name();
    if (!v->FromBinary(in)) in.setstate(istream::failbit);
    return *this;
  }

  // For pairs.
  template<typename T1, typename T2>
  BinaryDeSerializationByType& operator()(istream& in, pair<T1, T2>* v) {
    VLOG(5) << "pair:<" << typeid(T1).name() << ", " << typeid(T2).name() << ">";
    operator()(in, const_cast<typename std::remove_cv<T1>::type*>(&(v->first)));
    if (in.fail()) return *this;

    operator()(in, const_cast<typename std::remove_cv<T2>::type*>(&(v->second)));
    return *this;
  }

  // For Strings.
  // We specialize strings for optimization.
  template<typename T>
  BinaryDeSerializationByType& operator()(istream& in, basic_string<T>* v) {
    VLOG(5) << "string:" << typeid(T).name();
    v->clear();

    // Get the size of the string.
    size_t size = 0;
    operator()(in, &size);
    if (in.fail()) return *this;
    v->resize(size);
    in.read(const_cast<char*>(v->c_str()), size);
    return *this;
  }

  // For one dimension arrays.
  // Note: We currently do not support multi-dimension array.
  template<typename T, size_t N>
  BinaryDeSerializationByType& operator()(istream& in, T(*v)[N]) {
    VLOG(5) << "Arr:" << typeid(T).name();
    size_t size = 0;
    operator()(in, &size);
    if (in.fail()) return *this;

    ASSERT_EQ(size, N);
    for (size_t i = 0; i < N && !in.fail(); ++i)
      operator()(in, &((*v)[i]));
    return *this;
  }

  // ------------------------------------------------------------
  // Constructor
  // ------------------------------------------------------------
  BinaryDeSerializationByType(const BinaryDeSerializationParams& params =
      BinaryDeSerializationParams()) : params(params) {}

  BinaryDeSerializationParams params;
};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_BINARY_DESERIALIZATION_H_
