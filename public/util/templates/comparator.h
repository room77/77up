// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_TEMPLATES_COMPARATOR_H_
#define _PUBLIC_UTIL_TEMPLATES_COMPARATOR_H_

#include <functional>

namespace util {
namespace tl {

// Compares a specific member of the struct.
template<typename T, typename Member, Member T::*member, typename comparator = std::less<Member>>
struct compare_member : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    return comp(x.*member, y.*member);
  }
  comparator comp;
};

template<typename T, typename Member, Member T::*member>
struct less_member : compare_member<T, Member, member, std::less<Member>> {};

template<typename T>
struct less_first : less_member<T, decltype(T::first), &T::first> {};

template<typename T>
struct less_second : less_member<T, decltype(T::second), &T::second> {};

template<typename T, typename Member, Member T::*member>
struct greater_member : compare_member<T, Member, member, std::greater<Member>> {};

template<typename T>
struct greater_first : greater_member<T, decltype(T::first), &T::first> {};

template<typename T>
struct greater_second : greater_member<T, decltype(T::second), &T::second> {};

// Utility comparator that dereferences the objects to call the actual comparator.
template<typename T, typename comparator = std::less<T>>
struct comparator_dereference : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    return comp(*x, *y);
  }
  comparator comp;
};

template<typename T>
struct greater_first_second : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    return x.first > y.second;
  }
};

template<typename T>
struct greater_first_then_second : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    if (x.first > y.first) return true;
    return x.second > y.second;
  }
};

template<typename T>
struct less_first_second : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    return x.first < y.second;
  }
};

template<typename T>
struct less_first_then_second : std::binary_function <T, T, bool> {
  bool operator() (const T& x, const T& y) const {
    if (x.first < y.first) return true;
    return x.second < y.second;
  }
};



}  // namespace tl
}  // namespace util


#endif  // _PUBLIC_UTIL_TEMPLATES_COMPARATOR_H_
