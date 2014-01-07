// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

// Defines a base class for tasks. Tasks are prepped and/or launched using the
// run() method, and finalized using the wait(). Method.

// Tasks can be started in two ways (policy parameter):
// - launch::deferred preps the task but does not necessarily start. When you
//   call wait(). The task will start within the same thread.
// - laumch::async launches the task in background.

// In both approaches, wait() must be called once you are ready to receive the
// result of the task or make sure that it has finished.

// Tasks can be copied only if they are not currently running.
// Tasks must not be destroyed while they are being copied or running.

// Getting a multiple proxies to the same instance of the task through the
// factory's make_shared method does not allow the tasks to be run multiple
// times in paralle. If the instance detects that the task is running, run()
// will return false (task start failed).

// In order to use tasks in a server environment, you may want to look at
// task_server.h and task_server_example.cc

#ifndef _PUBLIC_UTIL_THREAD_TASK_H_
#define _PUBLIC_UTIL_THREAD_TASK_H_

#include <iostream>
#include <functional>
#include <atomic>
#include <future>

#include "../factory/factory.h"

namespace task {

template<class Function = std::function<bool()>>
class Task : public LazyFactory<Task<Function>> {
 public:
  typedef typename Function::result_type result_type;
  Task() = default;
  Task(Function func) : func_(func) {}

  virtual ~Task() {
    if (locked_.test_and_set()) {
      std::cerr << "Attempting to destroy a running task." << std::endl;
      abort();
    }
  }

  Task(const Task& t) { *this = t; }

  Task& operator = (const Task& t) {
    if (locked_.test_and_set() || t.locked_.test_and_set() ||
        running_ || t.running_) {
      std::cerr << "Locked tasks cannot be moved." << std::endl;
      abort();
    }
    func_ = t.func_;
    locked_.clear();
    t.locked_.clear();
    return *this;
  }

  // Returns true if the task was started. False otherwise.
  virtual bool run(const std::launch& policy = std::launch::deferred) const {
    if (!locked_.test_and_set()) {
      running_ = true;
      future_ = async(policy, func_);
      return true;
    }
    return false;
  }

  // wait for the result_type of the task and returns it's status.
  virtual result_type wait() const {
    future_.wait();
    running_ = false;
    locked_.clear();
    return future_.get();
  }

  // Returns true if this task is locked (typically means it is running).
  virtual bool Locked() const { return running_; }

 protected:
  mutable volatile std::atomic_flag locked_ = ATOMIC_FLAG_INIT;
  mutable bool running_ = false;
  mutable Function func_;
  mutable std::future<result_type> future_;
};

}

#endif  // _PUBLIC_UTIL_THREAD_TASK_H_
