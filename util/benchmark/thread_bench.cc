// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

// Simple threading library test.
// Requires GCC 4.6+ to compile.

// Creates 30k threads, each thread creating a proxy and calling inc / dec
// operators on the proxy 201 times in total per thread (modifying an
// atomic<int> variable).

// Set ulimit -s 256 before running.

// Average of 50 runs for Intel(R) Core(TM) i3-2100 CPU @ 3.10GHz:
// - with -O3: 465ms
// - with -O2: 487ms
// - no optimization: 495ms

#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <memory>
#include <deque>
#include <atomic>
#include "base/common.h"
#include "util/factory/factory.h"

FLAG_int(num_threads, 30000, "");

atomic<int> num(0);

class Base : public Factory<Base> {
 public:
  virtual void operator++() const { ++num; }
  virtual void operator--() const { --num; }
};

// Register a few instances to be more realistic.
auto register_base_0 = Base::bind("base0", [](){ return new Base; } );
auto register_base_1 = Base::bind("base1", [](){ return new Base; } );
auto register_base_2 = Base::bind("base2", [](){ return new Base; } );
auto register_base_3 = Base::bind("base3", [](){ return new Base; } );
auto register_base_4 = Base::bind("base4", [](){ return new Base; } );
auto register_base_5 = Base::bind("base5", [](){ return new Base; } );
auto register_base_6 = Base::bind("base6", [](){ return new Base; } );
auto register_base_7 = Base::bind("base7", [](){ return new Base; } );
auto register_base_8 = Base::bind("base8", [](){ return new Base; } );
auto register_base_9 = Base::bind("base9", [](){ return new Base; } );

void ThreadProxy(int i) {
  auto proxy = Base::make_shared("base1");
  ++*proxy;
  for (int i = 0; i < 100; ++i) {
    ++*proxy;
    --*proxy;
  }
}

void ThreadNoop(int i) {
}

void TestNewThread() {
  typedef std::chrono::high_resolution_clock Clock;
  Clock::time_point t0 = Clock::now();
  deque<thread> threads;
  for (int i = 0; i < gFlag_num_threads; ++i) {
    try {
      threads.push_back(thread(bind(ThreadProxy, i)));
    } catch (...) {
      cout << "Failed at " << i << endl;
      return;
    }
  }

  for (int i = 0; i < gFlag_num_threads; ++i) {
    threads[i].join();
  }

  Clock::time_point t1 = Clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

  cout << "Processed " << num << " threads in " << diff << " ms"<< endl;
}

int main() {
  TestNewThread();
  return 0;
}
