// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the serialization size tag for each type.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_TYPE_SIZE_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_TYPE_SIZE_H_

#include <type_traits>

#include "util/serial/types/varint.h"
#include "util/templates/sfinae.h"

namespace serial {

struct type_checkers {
  CREATE_MEMBER_FUNC_CHECK(get);
};

// -------------------------------------------------------------------
// Type Size.
// -------------------------------------------------------------------
enum {
  kSerialTypeSizeVarInt = 0,
  kSerialTypeSizeFour = 1,
  kSerialTypeSizeEight = 2,
  kSerialTypeUnknownVarInt = 3,  // The size is specified by a varint.
  kSerialTypeUnknownFixedInt = 4,  // The size is specified by a fixedint.
};

// Default implementation for all types.
template<typename T, typename = std::true_type>
struct SizeByType : std::integral_constant<int, kSerialTypeUnknownFixedInt> {};

// For all integer and enum types that are represented as Varints.
template<typename T>
struct SizeByType<T, std::integral_constant<bool,
    std::is_integral<T>::value  || std::is_enum<T>::value>>
        : std::integral_constant<int, kSerialTypeSizeVarInt> {};

// For floating types of size 4.
template<typename T>
struct SizeByType<T, std::integral_constant<bool,
    std::is_floating_point<T>::value && sizeof(T) == 4>>
        : std::integral_constant<int, kSerialTypeSizeFour> {};

// For floating types of size 8.
template<typename T>
struct SizeByType<T, std::integral_constant<bool,
    std::is_floating_point<T>::value && sizeof(T) == 8>>
        : std::integral_constant<int, kSerialTypeSizeEight> {};

// For varint.
template<typename T>
struct SizeByType<varint<T>>
    : std::integral_constant<int, kSerialTypeSizeVarInt> {};

// For fixed ints of size 4.
template<typename T>
struct SizeByType<fixedint<T>, std::integral_constant<bool,
    sizeof(T) == 4>> : std::integral_constant<int, kSerialTypeSizeFour> {};

// For fixed ints of size 8.
template<typename T>
struct SizeByType<fixedint<T>, std::integral_constant<bool,
    sizeof(T) == 8>> : std::integral_constant<int, kSerialTypeSizeEight> {};

// For strings.
template<>
struct SizeByType<string>
    : std::integral_constant<int, kSerialTypeUnknownVarInt> {};

// For pointer types.
template<typename T>
struct SizeByType<T, std::integral_constant<bool, std::is_pointer<T>::value>>
        : std::integral_constant<int,
              SizeByType<typename std::remove_pointer<T>::type>::value> {};

// For gettable types (e.g. shared_ptr, unique_ptr, etc.).
template<typename T>
struct SizeByType<T, std::integral_constant<bool,
    type_checkers::has_member_func_get<T>::value>> : std::integral_constant<int,
        SizeByType<decltype(reinterpret_cast<T*>(0)->get())>::value> {};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_TYPE_SIZE_H_
