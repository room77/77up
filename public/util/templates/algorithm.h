// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_TEMPLATES_ALGORITHM_H_
#define _PUBLIC_UTIL_TEMPLATES_ALGORITHM_H_

#include <algorithm>

#include "base/common.h"

namespace util {
namespace tl {

// Returns the iterator pointing to the element if binary search succeeds
// and false otherwise.
template<typename ForwardIterator, typename T>
ForwardIterator binary_search(ForwardIterator first, ForwardIterator last,
                              const T& value) {
  ForwardIterator iter = std::lower_bound(first, last, value);
  return (first != last && !(value < *first)) ? iter : last;
}

// Returns the iterator pointing to the element if binary search succeeds
// and false otherwise. Uses a custom comparator.
template<class ForwardIterator, class T, class Compare>
ForwardIterator binary_search(ForwardIterator first, ForwardIterator last,
                              const T& value, Compare comp) {
    ForwardIterator iter = std::lower_bound(first, last, value, comp);
    return (first != last && !comp(value, *first)) ? iter : last;
}

// Computes the set intersection on any container.
// This is useful for unordered containers.
// It will be faster to use std::set_intersection for sorted containers.
// Requires an insertion interator as out.
template <typename InputIt1, typename InputIt2, typename OutputIt>
OutputIt set_intersection(InputIt1 first1, InputIt1 last1,
                          InputIt2 first2, InputIt2 last2,
                          OutputIt out) {
  while (!(first1 == last1)) {
    if (!(std::find(first2, last2, *first1) == last2)) {
        *out = *first1;
        ++out;
    }
    ++first1;
  }
  return out;
}

}  // namespace tl
}  // namespace util


#endif  // _PUBLIC_UTIL_TEMPLATES_ALGORITHM_H_
