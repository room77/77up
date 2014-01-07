// Copyright 2013 Room77, Inc.
// Author: B. Uygar Oztekin

// TaskList is similar to Task but can retain multiple tasks that can be run
// in serial or parallel.

#ifndef _PUBLIC_UTIL_THREAD_TASK_LIST_H_
#define _PUBLIC_UTIL_THREAD_TASK_LIST_H_

#include <deque>
#include <functional>
#include "task.h"

namespace task {

template<class Function = std::function<void()>>
class TaskList : public Task<Function> {
 public:
  typedef typename Task<Function>::result_type result_type;
  TaskList() = default;

  void push_back(Function f) const {
    if (this->locked_.test_and_set()) {
      std::cerr << "You cannot modify a running task." << std::endl;
      abort();
    }
    tasks_.push_back(Task<Function>(f));
    this->locked_.clear();
  }

  // Start as many tasks as possible using the policy.
  // Return true iff all of them were started successfully, false otherwise.
  bool run(const std::launch& policy = std::launch::deferred) const {
    bool ret = true;
    if (!this->locked_.test_and_set()) {
      this->running_ = true;
      for (auto& task : tasks_) ret &= task.run(policy);
    }
    return ret;
  }

  // Wait for all tasks.
  result_type wait() const {
    for (auto& task : tasks_) task.wait();
    this->running_ = false;
    this->locked_.clear();
    return result_type();
  }

  size_t size() { return tasks_.size(); }

 protected:
  mutable std::deque<Task<Function>> tasks_;
};

}

#endif  // _PUBLIC_UTIL_THREAD_TASK_LIST_H_
