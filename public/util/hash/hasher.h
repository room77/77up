// Copyright 2013 Room77 Inc.
// Author: B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_HASH_HASHER_H_
#define _PUBLIC_UTIL_HASH_HASHER_H_

#include <cstdint>
#include <string>
#include "base/defs.h"
#include "util/factory/factory.h"

namespace hash {

// Base class that can be used to implement various hashers.
template<class Input, class Output>
class Hasher : public Factory<Hasher<Input, Output>> {
 public:
  typedef Input input_type;
  typedef Output output_type;

  virtual ~Hasher() {}

  virtual Output operator()(const Input&) const = 0;
};

}  // namespace hash

#endif  // _PUBLIC_UTIL_HASH_HASHER_H_
