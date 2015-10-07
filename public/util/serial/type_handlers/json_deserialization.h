// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the JSON deserialization for each type.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_JSON_DESERIALIZATION_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_JSON_DESERIALIZATION_H_

#include <istream>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "base/common.h"
#include "util/serial/encoding/encoding.h"
#include "util/serial/utils/serializer_util.h"
#include "util/serial/utils/stream_util.h"
#include "util/templates/sfinae.h"

namespace serial {

// Struct for setting default values based on type.
struct JSONDeSerializationByType {
  // Create all the checks.
  CREATE_MEMBER_FUNC_CHECK(clear);
  CREATE_MEMBER_TYPE_CHECK(value_type);
  CREATE_MEMBER_TYPE_CHECK(const_iterator);
  CREATE_MEMBER_FUNC_FIXED_SOFT_SIG_CHECK(typename T::iterator, insert,
                                          typename T::iterator,
                                          const typename T::value_type&);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(reset);
  CREATE_MEMBER_TYPE_CHECK(element_type);
  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(FromJSON);

  // Default implementation for specializations.
  // By default there is no specialization.
  template<typename T, typename = std::true_type>
  struct specializations : std::false_type {};

  // All specializations. Any specializations should be added here to avoid
  // ambiguous function instantitaion for different types.
  template<typename T>
  struct specializations<T, std::integral_constant<bool,
      std::is_pointer<T>::value ||
      (has_member_func_sig_FromJSON<T,
          bool (istream&, const JSONDeSerializationParams&)>::value) ||
      has_member_func_sig_FromJSON<T, bool (istream&)>::value ||
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
      !specializations<T>::value, JSONDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "UnSupported: " << typeid(T).name();
    ASSERT(0) << "Unsupported type: " << typeid(T).name();;
    return *this;
  }

  // For all serializable types with size > 1.
  template<typename T>
  typename std::enable_if<is_serializable<T>::value && sizeof(T) != 1,
      JSONDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "basic: " << typeid(T).name() << ", size: " << sizeof(T);
    ASSERT_NOTNULL(v);
    util::SkipSpaces(in);

    in >> *v;

    if (in.fail())
      util::LogParsingError(in, -1,
                            string("Could not parse basic type: ") + typeid(T).name(),
                            params.err);
    return *this;
  }

  // For all serializable types with size == 1.
  template<typename T>
  typename std::enable_if<is_serializable<T>::value && sizeof(T) == 1,
      JSONDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "char: " << typeid(T).name() << ", size: " << sizeof(T);
    unsigned short temp = 0;
    operator()(in, &temp);
    *v = static_cast<T>(temp);
    return *this;
  }

  // For all pointer types.
  template<typename T>
  typename std::enable_if<std::is_pointer<T>::value,
      JSONDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "pointer: " << typeid(T).name();

    T new_val = new typename std::remove_pointer<T>::type();
    *v = new_val;
    operator()(in, new_val);

    return *this;
  }

  // For all types that are iterable.
  template<typename T>
  typename std::enable_if<has_member_func_clear<T>::value &&
      has_member_type_value_type<T>::value && has_member_type_const_iterator<T>::value &&
      has_member_func_fixed_sig_insert<T>::value,
      JSONDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    // Read the container's size, then its content.
    VLOG(5) << "container:" << typeid(T).name();
    v->clear();
    bool expect_multiple = (util::SkipToNextChar(in) == '[');
    if (expect_multiple && !util::ExpectNext(in, "[", true, params.err))
      return *this;
    while (in.good() && util::SkipToNextChar(in) != ']' && in.good()) {
      typename T::value_type value;
      operator()(in, &value);
      if (in.fail()) break;
      v->insert(v->end(), value);

      if (!expect_multiple) break;

      // Next char can either be ',' or ']'.
      char ch = util::ExpectNext(in, ",]", false, params.err);
      if (!ch) break;
      else if (ch == ',') in.get();
    }

    if (in.fail() || (expect_multiple &&
        !util::ExpectNext(in, "]", true, params.err))) return *this;

    return *this;
  }

   // For all types that have a 'reset' function and have 'element_type'.
   template<typename T>
   typename std::enable_if<has_member_func_sig_reset<T, void()>::value &&
       has_member_type_element_type<T>::value, JSONDeSerializationByType&>::type
   operator()(istream& in, T* v) {
     VLOG(5) << "ptr:" << typeid(T).name();
     v->reset(new typename T::element_type());
     operator()(in, v->get());
     return *this;
   }

  // For all nested structs.
  template<typename T>
  typename std::enable_if<
      has_member_func_sig_FromJSON<T, bool (istream&, const JSONDeSerializationParams&)>::value,
      JSONDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "struct: " << typeid(T).name();
    if (!v->FromJSON(in, params)) in.setstate(istream::failbit);
    return *this;
  }

  // For all custom types that implement FromJSON for deserialization.
  template<typename T>
  typename std::enable_if<
      !has_member_func_sig_FromJSON<T, bool (istream&, const JSONDeSerializationParams&)>::value &&
      has_member_func_sig_FromJSON<T, bool (istream&)>::value,
      JSONDeSerializationByType&>::type
  operator()(istream& in, T* v) {
    VLOG(5) << "struct: " << typeid(T).name();
    if (!v->FromJSON(in)) in.setstate(istream::failbit);
    return *this;
  }

  // For pairs.
  template<typename T1, typename T2>
  JSONDeSerializationByType& operator()(istream& in,
                                       pair<T1, T2>* v) {
    VLOG(5) << "pair:<" << typeid(T1).name() << ", " << typeid(T2).name() << ">";

    if (!util::ExpectNext(in, "[", true, params.err)) return *this;

    operator()(in, const_cast<typename std::remove_cv<T1>::type*>(&(v->first)));

    if (in.fail() || !util::ExpectNext(in, ",", true, params.err)) return *this;

    operator()(in, const_cast<typename std::remove_cv<T2>::type*>(&(v->second)));

    if (in.fail() || !util::ExpectNext(in, "]", true, params.err)) return *this;

    return *this;
  }

  // For Strings.
  template<typename T>
  JSONDeSerializationByType& operator()(istream& in, basic_string<T>* v) {
    VLOG(5) << "string:" << typeid(T).name();
    v->clear();
    static const string delims = " ,]}\n\r:";
    encoding::UnEscapeQuotedStringFromStream(in, v, delims);
    return *this;
  }

  // For one dimension arrays.
  // Note: We currently do not support multi-dimension array.
  template<typename T, size_t N>
  JSONDeSerializationByType& operator()(istream& in, T(*v)[N]) {
    VLOG(5) << "Arr:" << typeid(T).name();
    int count = 0;

    bool expect_multiple = (util::SkipToNextChar(in) == '[');
    if (expect_multiple && !util::ExpectNext(in, "[", true, params.err))
      return *this;

    while (in.good() && util::SkipToNextChar(in) != ']' && in.good()) {
      operator()(in, &((*v)[count++]));
      if (in.fail()) break;

      if (!expect_multiple) break;

      // Next char can either be ',' or ']'.
      char ch = util::ExpectNext(in, ",]", false, params.err);
      if (!ch) break;
      else if (ch == ',') in.get();
    }

    // TODO(pramodg): Call setdefault on rest of elements in array.

    if (in.fail() || (expect_multiple &&
        !util::ExpectNext(in, "]", true, params.err))) return *this;

    return *this;
  }

  // Special rule for unordered_map<string, T>.
   // This is serialized in the form { key : value , key : value, ... }
   template<class T>
   JSONDeSerializationByType& operator()(istream& in,
                                         unordered_map<string, T>* v) {
     VLOG(5) << "unordered_map:" << typeid(T).name();

     if (!util::ExpectNext(in, "{", true, params.err)) return *this;

     while (in.good() && util::SkipToNextChar(in) != '}' && in.good()) {
       string key;
       operator()(in, &key);

       // Find ':'
       if (in.fail() || !util::ExpectNext(in, ":", true, params.err)) break;

       T value;
       operator()(in, &value);
       if (in.fail()) break;

       v->insert(make_pair(key, value));

       // Next char can either be ',' or '}'.
       char ch = util::ExpectNext(in, ",}", false, params.err);
       if (!ch) break;
       else if (ch == ',') in.get();
     }

     if (in.fail() || !util::ExpectNext(in, "}", true, params.err)) return *this;

     return *this;
   }

   // For bool.
   JSONDeSerializationByType& operator()(istream& in, bool* v) {
     VLOG(5) << "bool";
     util::SkipSpaces(in);
     static const string delims = " ,]}\n\r";
     string str = util::ExtractDelimited(in, delims);

     if (str == "true" || str == "1") *v = true;
     else *v = false;

     return *this;
   }

  // ------------------------------------------------------------
  // Constructor
  // ------------------------------------------------------------
  JSONDeSerializationByType(const JSONDeSerializationParams& params =
      JSONDeSerializationParams()) : params(params) {}

  JSONDeSerializationParams params;
};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_JSON_DESERIALIZATION_H_
