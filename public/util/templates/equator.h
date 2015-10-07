// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_TEMPLATES_EQUATOR_H_
#define _PUBLIC_UTIL_TEMPLATES_EQUATOR_H_

#include <functional>

namespace util {
namespace tl {

// Compares a specific member of the struct.
template<typename T, typename Member, Member T::*member, typename equator = std::equal_to<Member>>
struct equal_member : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    return eq(x.*member, y.*member);
  }
  equator eq;
};

// Compares T::first.
template<typename T>
struct equal_first : equal_member<T, decltype(&T::first), &T::first> {};

// Compares T::second.
template<typename T>
struct equal_second : equal_member<T, decltype(&T::second), &T::second> {};

// Utility equator that dereferences the objects to call the actual equator.
template<typename T, typename equator = std::equal_to<T>>
struct equal_dereference : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    return eq(*x, *y);
  }
  equator eq;
};

}  // namespace tl
}  // namespace util


#endif  // _PUBLIC_UTIL_TEMPLATES_EQUATOR_H_
