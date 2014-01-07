// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: edelman@room77.com (Nicholas Edelman)

// Rate limited thread pool.
// This ensures a max qps is maintained at all times.

#ifndef _PUBLIC_UTIL_THREAD_RATE_LIMITED_THREAD_POOL_H_
#define _PUBLIC_UTIL_THREAD_RATE_LIMITED_THREAD_POOL_H_

#include "base/common.h"
#include "util/thread/rate_limited_shared_queue.h"
#include "util/thread/thread_pool.h"

namespace util {
namespace threading {

// Creates a new threadpool of 'n' threads.
class RateLimitedThreadPool : public ThreadPool {
 public:
  // this thread pool does NOT process when there is free capacity. rather
  // the thread pool will process at a maximum of the desired qps parameters
  // up to the capacity of the thread pool
  // @param qps - qps is NOT an average qps. It is a max qps. For instance, if you
  //   specify qps=2, then at most one task can be executed every 500ms.
  RateLimitedThreadPool(double qps, int num_workers,
                        int capacity = numeric_limits<int>::max())
      : ThreadPool(SQ(new RateLimitedSharedQueue<ThreadPoolFunc>(qps, 1, capacity)),
                   num_workers, capacity) {}
};

}  // namespace threading
}  // namespace util


#endif  // _PUBLIC_UTIL_THREAD_RATE_LIMITED_THREAD_POOL_H_
