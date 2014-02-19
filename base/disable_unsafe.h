// Copyright 2014 Room77, Inc.
// Author: Uygar Oztekin

// This file contains items we chose to disable to avoid potential issues.

#ifndef _PUBLIC_BASE_DISABLE_UNSAFE_H_
#define _PUBLIC_BASE_DISABLE_UNSAFE_H_

#include <functional>

// Disable const char* default hash implementation as, in most cases, it is not
// what the user may want. std::hash has a catch all pointer implementation
// that covers const char*. If you need to hash const char* as a "string",
// explicitly use ::hash::str_hash in util/hash/hash_util.h.
namespace std {
template<>
struct hash<const char*> {
  inline size_t operator()(const char *s) const __attribute__ ((deprecated)) {
    void* ptr = 0;
    *static_cast<int*>(ptr) = 0;  // Cause a segfault.
    return 0;
  }
};
}

#endif  // _PUBLIC_BASE_DISABLE_UNSAFE_H_
