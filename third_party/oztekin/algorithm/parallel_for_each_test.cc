// Copyright 2014 B. Uygar Oztekin

#include <cassert>
#include <vector>
#include "parallel_for_each.h"

int main() {
  for (int size = 0; size < 1000; ++size) {
    for (int parallelism = 0; parallelism < 10; ++parallelism) {
      // Create a vector of zeros.
      std::vector<int> v(size, 0);
      assert(v.size() == size);
      // Increment all entries by one in parallel.
      parallel_for_each(v.begin(), v.end(), [](int& i){ ++i; }, parallelism);
      // Verify that all entries are now 1.
      for (const auto& i : v) assert(i == 1);
    }
  }
  return 0;
}
