// Copyright 2013 Room77 Inc.
// Author: B. Uygar Oztekin

#include <string>
#include "util/hash/hasher.h"

namespace hash {

template<class Output>
class Stl : public Hasher<string, Output> {
 public:
  // Implementations for 64 and 128 bits.
  Output operator()(const string& str) const {
    return Output(std::hash<string>()(str));
  }
};

namespace {

auto stl_32  = Hasher<string, uint32_t> ::bind("stl", []{ return new Stl<uint32_t>; });
auto stl_64  = Hasher<string, uint64_t> ::bind("stl", []{ return new Stl<uint64_t>; });
auto stl_128 = Hasher<string, uint128_t>::bind("stl", []{ return new Stl<uint128_t>; });

}  // namespace

}  // namespace hash
