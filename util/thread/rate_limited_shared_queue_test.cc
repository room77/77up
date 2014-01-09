// Copyright 2013 Room77, Inc.
// Author: Nicholas Edelman (edelman)

#include <thread>

#include "test/cc/test_main.h"
#include "util/thread/rate_limited_shared_queue.h"

namespace util {
namespace threading {
namespace test {

uint64_t TimestampMs() {
  return chrono::duration_cast<chrono::milliseconds>(
    chrono::high_resolution_clock::now().time_since_epoch()).count();
}

template <typename T>
void AddValues(RateLimitedSharedQueue<T>& rls_queue, T default_val, int nvals) {
  for (int i = 0; i < nvals; ++i) {
    rls_queue.push(default_val);
  }
}

template <typename T>
void ConsumeVals(RateLimitedSharedQueue<T>& rls_queue, int nvals) {
  for (int i = 0; i < nvals; ++i) {
    rls_queue.consume_batch();
  }
}

TEST(RateLimitedSharedQueue, CanRetrieveImmediately) {
  const double num_vals = 100;
  const double ms_per_query = 1000;
  RateLimitedSharedQueue<int> rls_queue(1000.0 / ms_per_query, 1);
  AddValues(rls_queue, 0, num_vals);
  uint64_t start_ms = TimestampMs();
  rls_queue.consume_batch();
  double measured_ms_query = TimestampMs() - start_ms;
  // verify the read happens in much less than the 1000 ms per query
  EXPECT_TRUE(measured_ms_query < 10);
}

TEST(RateLimitedSharedQueue, AddValuesVerifyRate) {
  const double num_vals = 100;
  const double ms_per_query = 10;
  RateLimitedSharedQueue<int> rls_queue(1000.0 / ms_per_query, 1);
  AddValues(rls_queue, 0, num_vals);

  uint64_t start_ms = TimestampMs();
  for (int i = 0; i < num_vals; ++i) {
    rls_queue.consume_batch();
  }
  double measured_ms_per_query = (TimestampMs() - start_ms) / num_vals;
  EXPECT_GT(measured_ms_per_query, 9.8);
  EXPECT_LT(measured_ms_per_query, 10.0);
}


TEST(RateLimitedSharedQueue, MultipleConsumersAddValuesVerifyRate) {
  const uint64_t num_vals = 1e2;
  const double ms_per_query = 10;
  const double num_threads = 4;
  RateLimitedSharedQueue<int> rls_queue(1000.0/ms_per_query, 1);
  vector<thread> ths;
  for (int i = 0; i < num_threads; ++i) {
    ths.push_back(thread(std::bind(AddValues<int>, ref(rls_queue), 0, num_vals)));
  }
  for (auto& th : ths) th.join();

  uint64_t start_ms = TimestampMs();
  ths.clear();
  for (int i = 0; i < num_threads; ++i) {
    ths.push_back(thread(std::bind(ConsumeVals<int>, ref(rls_queue), num_vals)));
  }
  for (auto& th : ths) th.join();
  double measured_ms_per_query = (TimestampMs() - start_ms) / (num_threads*num_vals);
  EXPECT_GT(measured_ms_per_query, 9.9);
  EXPECT_LT(measured_ms_per_query, 10.1);
}

} // namespace test
} // namespace threading
} // namespace util
