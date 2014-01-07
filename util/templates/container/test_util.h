// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_TEMPLATES_CONTAINER_TEST_UTIL_H_
#define _PUBLIC_UTIL_TEMPLATES_CONTAINER_TEST_UTIL_H_

#include <random>

#include "base/defs.h"

namespace tl {
namespace test {

vector<int> GetNShuffledNumbers(int n = 10) {
  std::vector<int> numbers(n);
  for (int i = 0; i < n; ++i) numbers[i] = i;
  std::shuffle(numbers.begin(), numbers.end(), std::mt19937(std::random_device()()));
  return numbers;
}

}  // namespace test
}  // namespace tl


#endif  // _PUBLIC_UTIL_TEMPLATES_CONTAINER_TEST_UTIL_H_
