// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Retrieves the strack trace for all threads in a process.

#ifndef _PUBLIC_UTIL_THREAD_THREAD_STACK_H_
#define _PUBLIC_UTIL_THREAD_THREAD_STACK_H_

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>

#include "base/common.h"

#include "util/thread/counters.h"

namespace util {
namespace threading {

class ThreadStack {
 public:
  ~ThreadStack() {}
  static ThreadStack& Instance() {
    static ThreadStack the_one;
    return the_one;
  }

  // Returns the stack trace for a given list of thread ids.
  // Note that this uses signals to get the stack trace of all threads and
  // some syscalls return error if they are interrupted.
  // TODO(pramodg): This is unsafe if we use TCMALLOC. If the signal is raised when one of the
  // threads is already holding a mutex inside tcmalloc, all threads will potentailly deadlock
  // when they try to allocate any new memory as tcmalloc does not support recursive mutex locking.
  // Maybe use similar code as https://code.google.com/p/gperftools/source/browse/src/profiler.cc
  // to get this to work.
  string GetTraceForThreadIds(const vector<int>& thread_ids);

  // Returns the stack trace for all threads.
  string GetTraceForAllThreads();

  // Used by the signall handler to print the stack trace of the current thread.
  void AddTraceForCurrentThread();

 private:
  ThreadStack() {}

  map<string, set<std::thread::id> > stacks_;
  mutex stacks_mutex_;
  mutex mutex_;

  Counter counter_;
};

}  // namespace threading
}  // namespace util


#endif  // _PUBLIC_UTIL_THREAD_THREAD_STACK_H_
