// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

// You may want to disable swap to avoid thrashing.

#include <iostream>
#include <memory>

#include "base/common.h"

constexpr int alloc_size = 1024 * 1024 * 1024;
constexpr int gigabytes = 4;    // Increase this to a large number to test.

void Test() {
  cout << "Attempting to allocate " << gigabytes << "x 1GB" << endl;
  // We will leak memory for this test. This is fine.
  for (int i = 0; i < gigabytes; ++i) {
    char* p = new char[alloc_size];
    // Touch the pages so that they will actually be allocated.
    for (int k = 0; k < alloc_size; k += 1024) p[k] = '\0';
  }
  cout << "Success" << endl;
}

int main() {
  Test();
  return 0;
}
