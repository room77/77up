// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Utility thread pools used for various tasks in the binary.

#ifndef _PUBLIC_UTIL_THREAD_UTILITY_THREAD_POOLS_H_
#define _PUBLIC_UTIL_THREAD_UTILITY_THREAD_POOLS_H_


#include "util/thread/thread_pool.h"

extern int gFlag_non_time_critical_tasks_pool_size;

namespace util {
namespace threading {

// Returns a thread pool that can be used for all non time critical tasks.
inline  ThreadPoolFactory::mutable_shared_proxy& NonTimeCriticalTasksPool() {
  static ThreadPoolFactory::mutable_shared_proxy proxy =
      ThreadPoolFactory::Pool("non_time_critical_tasks",
                              gFlag_non_time_critical_tasks_pool_size);
  return proxy;
}

}  // namespace threading
}  // namespace util

#endif  // _PUBLIC_UTIL_THREAD_UTILITY_THREAD_POOLS_H_
