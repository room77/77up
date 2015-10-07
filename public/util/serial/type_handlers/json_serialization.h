// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the JSON serialization for each type.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_JSON_SERIALIZATION_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_JSON_SERIALIZATION_H_

#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "base/common.h"
#include "util/serial/encoding/encoding.h"
#include "util/serial/utils/serializer_util.h"
#include "util/templates/sfinae.h"

namespace serial {

// Struct for setting default values based on type.
struct JSONSerializationByType {
  // Create all the checks.
  CREATE_MEMBER_FUNC_CHECK(get);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(size);
  CREATE_MEMBER_TYPE_CHECK(const_iterator);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(ToJSON);

  // Default implementation for specializations.
  // By default there is no specialization.
  template<typename T, typename = std::true_type>
  struct specializations : std::false_type {};

  // All specializations. Any specializations should be added here to avoid
  // ambiguous function instantitaion for different types.
  template<typename T>
  struct specializations<T, std::integral_constant<bool,
      std::is_pointer<T>::value ||
      (has_member_func_sig_ToJSON<T,
          void(ostream&, const JSONSerializationParams&)>::value) ||
      has_member_func_sig_ToJSON<T, void(ostream&)>::value ||
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

  // For all integer types with size > 1.
  template<typename T>
  typename std::enable_if<(is_integral<T>::value  || std::is_enum<T>::value),
      JSONSerializationByType&>::type
  operator()(ostream& out, const T v) {
    VLOG(5) << "basic: " << typeid(T).name() << ", size: " << sizeof(T);
    T temp = v;
    if (is_signed<T>::value &&  temp < 0) {
      out.put('-');
      temp = -temp;
    }

    char buffer[numeric_limits<T>::digits10 + 1];
    int i = numeric_limits<T>::digits10;
    do {
      buffer[i--] = (temp % 10) + '0';
      temp /= 10;
    } while (temp > 0);
    ++i;
    out.write(buffer + i, numeric_limits<T>::digits10 - i + 1);
    return *this;
  }

  // For all floating types.
  template<typename T>
  typename std::enable_if<is_floating_point <T>::value,
      JSONSerializationByType&>::type
  operator()(ostream& out, const T v) {
    VLOG(5) << "basic: " << typeid(T).name() << ", size: " << sizeof(T);
    if(std::isnormal(v)) {
      stringstream ss;
      ss << setprecision(numeric_limits<T>::digits10) << v;
      const string str = ss.str();
      out.write(str.c_str(), str.size());

      /*
      // We need a lot of precision to get lat-longs right. sprintf just cannot
      // handle all the cases we need to work. We also do not want to use out <<
      // directly as we may change ostream in the future and want to reduce the
      // number of interface functions for the new basestream class.
      static const int kBufferSize = numeric_limits<T>::max_digits10 + 30;
      char buffer[kBufferSize];
      int ret = snprintf(buffer, kBufferSize, "%.15g", v);
      out.write(buffer, ret);
       */
    } else
      out.put('0');

    return *this;
  }

  // For all pointer types.
  template<typename T>
  typename std::enable_if<std::is_pointer<T>::value,
      JSONSerializationByType&>::type
  operator()(ostream& out, const T v) {
    VLOG(5) << "pointer: " << typeid(T).name();
    if (v != nullptr)
      operator()(out, *v);
    else {
      typename std::remove_pointer<T>::type def =
          typename std::remove_pointer<T>::type();
      operator()(out, def);
    }
    return *this;
  }

  // For all types that are iterable.
  // This is serialized in the form [ value, value, ... ]
  template<typename T>
  typename std::enable_if<has_member_func_sig_size<T, size_t ()>::value &&
      has_member_type_const_iterator<T>::value, JSONSerializationByType&>::type
  operator()(ostream& out, const T& v) {
    VLOG(5) << "container:" << typeid(T).name() << ", size: " << v.size();
    AddSeparator(out, '[');
    params.Indent();
    VLOG(5) << params.indent;
    int count = 0;
    for (const auto& elem : v) {
      if (count++ > 0) AddSeparator(out, ',');
      AddLine(out);
      operator()(out, elem);
    }
    params.Dedent();
    AddLine(out);
    AddSeparator(out, ']');
    return *this;
  }

  // For all types that have a 'get' function.
  template<typename T>
  typename std::enable_if<has_member_func_get<T>::value,
      JSONSerializationByType&>::type
  operator()(ostream& out, const T& v) {
    VLOG(5) << "Gettable:" << typeid(T).name();
    operator()(out, v.get());
    return *this;
  }

  // For all nested structs.
  template<typename T>
  typename std::enable_if<
      has_member_func_sig_ToJSON<T, void(ostream&, const JSONSerializationParams&)>::value,
      JSONSerializationByType&>::type
  operator()(ostream& out, const T& v) {
    VLOG(5) << "struct: " << typeid(T).name();
    v.ToJSON(out, params);
    return *this;
  }

  // For all custom types that implement only the ToJSON function for serialization.
  template<typename T>
  typename std::enable_if<
      !has_member_func_sig_ToJSON<T, void(ostream&, const JSONSerializationParams&)>::value &&
      has_member_func_sig_ToJSON<T, void(ostream&)>::value,
      JSONSerializationByType&>::type
  operator()(ostream& out, const T& v) {
    VLOG(5) << "struct: " << typeid(T).name();
    v.ToJSON(out);
    return *this;
  }

  // For pairs.
  template<typename T1, typename T2>
  JSONSerializationByType& operator()(ostream& out, const pair<T1, T2>& v) {
    VLOG(5) << "pair:<" << typeid(T1).name() << ", " << typeid(T2).name() << ">";

    AddSeparator(out, '[');
    params.Indent();
    AddLine(out);
    operator()(out, v.first);
    AddSeparator(out, ',');
    AddLine(out);
    operator()(out, v.second);
    params.Dedent();
    AddLine(out);
    AddSeparator(out, ']');
    return *this;
  }

  // For Strings.
  // We specialize strings for optimization.
  template<typename T>
  JSONSerializationByType& operator()(ostream& out,
                                      const basic_string<T>& v) {
    VLOG(5) << "string:" << typeid(T).name() << ", size: " << v.size();
    encoding::EscapeStringToStream(out, v, true);
    return *this;
  }

  // For one dimension arrays.
  // Note: We currently do not support multi-dimension array.
  template<typename T, std::size_t N>
  JSONSerializationByType& operator()(ostream& out, const T(&v)[N]) {
    VLOG(5) << "Arr:" << typeid(T).name();
    AddSeparator(out, '[');
    params.Indent();
    int count = 0;
    for (size_t i = 0; i < N; ++i) {
      if (count++ > 0) AddSeparator(out, ',');
      AddLine(out);
      operator()(out, v[i]);
    }
    params.Dedent();
    AddLine(out);
    AddSeparator(out, ']');
    return *this;
  }

  // Special rule for unordered_map<string, T>.
  // This is serialized in the form { key : value , key : value, ... }
  template<class T>
  JSONSerializationByType& operator()(ostream& out,
                                      const unordered_map<string, T>& v) {
    VLOG(5) << "unordered_map:" << typeid(T).name() << ", size: " << v.size();

    // Write out the container's content as an associative array.
    AddSeparator(out, '{');
    params.Indent();
    int count = 0;
    for (const auto& p : v) {
      if (count++ > 0) AddSeparator(out, ',');
      AddLine(out);
      operator()(out, p.first);
      AddSeparator(out, ':', true);
      operator()(out, p.second);
    }
    params.Dedent();
    AddLine(out);
    AddSeparator(out, '}');
    return *this;
  }

  // For bool.
  JSONSerializationByType& operator()(ostream& out, const bool v) {
    VLOG(5) << "bool";
    if (v) out.write("true", 4);
    else out.write("false", 5);
    return *this;
  }

  // ------------------------------------------------------------
  // Utility functions.
  // ------------------------------------------------------------
  // Add indentation, if requested.
  void AddLine(ostream& out) {
    if (params.indent_increment > 0) {
      out.put('\n');
      string s(params.indent, ' ');
      out.write(s.c_str(), s.size());
    }
  }

  void AddSpace(ostream& out) {
    if (params.indent_increment > 0) out.put(' ');
  }

  void AddSeparator(ostream& out, const char sep, bool space = false) {
    out.put(sep); if (space) AddSpace(out);
  }

  // ------------------------------------------------------------
  // Constructor
  // ------------------------------------------------------------
  JSONSerializationByType(const JSONSerializationParams& params =
      JSONSerializationParams()) : params(params) {}

  JSONSerializationParams params;
};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_JSON_SERIALIZATION_H_
