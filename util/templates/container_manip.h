// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Utility manipulators for containers.

#ifndef _PUBLIC_UTIL_TEMPLATES_CONTAINER_MANIP_H_
#define _PUBLIC_UTIL_TEMPLATES_CONTAINER_MANIP_H_

#include "util/string/strutil.h"
#include "util/templates/container_util.h"

namespace util {
namespace tl {

// Generates a key value container from an iterable container, where the value
// for each key is the order of first occurence in the iterable container.
template<typename KVContainer, typename IterableContainer>
KVContainer GetContainerWithOrderIndex(const IterableContainer& input) {
  KVContainer res;
  int i = 0;
  for (const auto& key: input) FindWithInsert(res, key, i++);
  return res;
}

// Generates a key value container from an iterable container, where the value
// for each key is the order of first occurence in the separable string.
template<typename KVContainer>
KVContainer GetContainerWithOrderIndex(const string& input,
                                       const string& separator = ",") {
  vector<string> c;
  strutil::SplitString(input, ",", &c);
  return GetContainerWithOrderIndex<KVContainer, vector<string> >(c);
}

}  // namespace tl
}  // namespace util

#endif  // _PUBLIC_UTIL_TEMPLATES_CONTAINER_MANIP_H_
