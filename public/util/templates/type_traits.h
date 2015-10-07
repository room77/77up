// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_TEMPLATES_TYPE_TRAITS_H_
#define _PUBLIC_UTIL_TEMPLATES_TYPE_TRAITS_H_

#include <type_traits>

namespace util {
namespace tl {

// Removes reference const and volatile.
template< class T >
struct remove_rcv {
    typedef typename std::remove_volatile<typename std::remove_const<
        typename std::remove_reference<T>::type>::type>::type type;
};

// Returns true it the type is char.
template< class T>
struct is_char_type : std::integral_constant<bool,
        std::is_same<char, typename std::remove_cv<T>::type>::value ||
        std::is_same<unsigned char, typename std::remove_cv<T>::type>::value > {};

}  // namespace tl
}  // namespace util

#endif  // _PUBLIC_UTIL_TEMPLATES_TYPE_TRAITS_H_
