// Copyright 2013 Room77 Inc.
// Author: B. Uygar Oztekin

#include <string>
#include "util/hash/hasher.h"
#include "util/hash/murmur3/murmur3_hash.h"

namespace hash {

template<class Output>
class Murmur3 : public Hasher<string, Output> {
 public:
  // Implementations for 64 and 128 bits.
  Output operator()(const string& str) const {
    uint128_t ret;
    MurmurHash3_x86_128(&str[0], str.size(), 0, &ret);
    return Output(ret);
  }
};

// Faster specialization for uint32_t.
template<>
uint32_t Murmur3<uint32_t>::operator()(const string& str) const {
  uint32_t ret;
  MurmurHash3_x86_32(&str[0], str.size(), 0, &ret);
  return ret;
}

namespace {

auto mm3_32  = Hasher<string, uint32_t> ::bind("murmur3", []{ return new Murmur3<uint32_t>; });
auto mm3_64  = Hasher<string, uint64_t> ::bind("murmur3", []{ return new Murmur3<uint64_t>; });
auto mm3_128 = Hasher<string, uint128_t>::bind("murmur3", []{ return new Murmur3<uint128_t>; });

}  // namespace

}  // namespace hash

