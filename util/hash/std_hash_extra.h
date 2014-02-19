// Copyright 2011 Room77 Inc.
// Author: B. Uygar Oztekin

// Extra hashing functions for common types that are pretty safe to be included
// to C++ std library. If they are added in the future, we will remove them to
// avoid multiple definitions.

#ifndef _PUBLIC_UTIL_HASH_STD_HASH_EXTRA_H_
#define _PUBLIC_UTIL_HASH_STD_HASH_EXTRA_H_

namespace std {

// Hash pair type
template<typename T1, typename T2>
struct hash<pair<T1, T2> > {
 public:
  hash() : hash_t1(), hash_t2() {}

  size_t operator()(const pair<T1, T2> &p) const {
    size_t seed = hash_t1(p.first);
    return hash_t2(p.second) + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

 private:
  std::hash<T1> hash_t1;
  std::hash<T2> hash_t2;
};

}

#endif  // _PUBLIC_UTIL_HASH_STD_HASH_EXTRA_H_
