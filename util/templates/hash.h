// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_TEMPLATES_HASH_H_
#define _PUBLIC_UTIL_TEMPLATES_HASH_H_

#include <functional>
#include <memory>

#include "base/defs.h"

namespace util {
namespace tl {

// No hashing.
template<typename T>
struct identity_hash {
  size_t operator()(const T& v) const {
    return static_cast<size_t>(v);
  }
};

// A fast hash function for small datasets.
struct fast_hash {
  size_t operator()(const char* v, int len) const {
    size_t hash = 13;
    for (; len >= sizeof(int); len -= sizeof(int), v += sizeof(int))
      hash = hash * 101 + *(reinterpret_cast<const int*>(v));

    for (; len >= sizeof(short); len -= sizeof(short), v += sizeof(short))
      hash = hash * 101 + *(reinterpret_cast<const short*>(v));

    for (; len >= sizeof(char); len -= sizeof(char), v += sizeof(char))
      hash = hash * 101 + *v;
    return hash;
  }

  template<typename T>
  size_t operator()(const T& v) const {
    return operator ()(reinterpret_cast<const char*>(&v), sizeof(T));
  }

  size_t operator()(const string& v) const {
    return operator ()(v.c_str(), v.size());
  }
};

// Hashes a specific member of a struct.
template<typename T, typename Member, Member T::*member, typename hasher = std::hash<Member>>
struct hash_member {
  size_t operator()(const T& v) const {
    return h(v.*member);
  }
  hasher h;
};

// Utility Hasher that dereferences the object to call the actual hasher.
template<typename T, typename hasher = std::hash<T>>
struct hash_dereference {
  size_t operator()(const T& v) const {
    return h(*v);
  }
  hasher h;
};

}  // namespace tl
}  // namespace util


#endif  // _PUBLIC_UTIL_TEMPLATES_HASH_H_
