// Copyright 2013 Room77, Inc.
// Author: Nicholas Edelman (edelman)

//
// Adds rate limiting functionality to the shared queue
// The QPS is not an average QPS. It is a max qps.
// For instance, if you specify QPS=2, then at most
// one task can be executed every 500ms
//

#ifndef _PUBLIC_UTIL_THREAD_RATE_LIMITED_SHARED_QUEUE_H_
#define _PUBLIC_UTIL_THREAD_RATE_LIMITED_SHARED_QUEUE_H_

#include <chrono>
#include <thread>
#include <queue>

#include "base/common.h"
#include "util/thread/shared_queue.h"

namespace util {
namespace threading {

template <typename T>
class RateLimitedSharedQueue : public SharedQueue<T> {
 public:
  RateLimitedSharedQueue(double qps, int batch_size = 1,
                         int capacity = numeric_limits<int>::max(),
                         int notify_timeout = -1)
    : SharedQueue<T>(batch_size, capacity, notify_timeout),
      min_ms_between_tasks_(1000.0 / qps),
      // set to -2*min_ms_between_tasks in the past, so the new task can be added immediately
      last_task_add_timestamp_ms_(chrono::duration_cast<chrono::milliseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count() - 2*min_ms_between_tasks_),
      run_qps_thread_(true) {
      consume_qps_thread_.reset(
          new thread(&RateLimitedSharedQueue<T>::ConsumeQueryRate, this));
    }

  virtual ~RateLimitedSharedQueue() {
    if (consume_qps_thread_) {
      run_qps_thread_ = false;
      consume_qps_thread_->join();
    }
  }

  virtual void push(const T& t) override {
    T task;
    bool add_task = false;
    {
      lock_guard<mutex> l(m_);
      // add the new task
      tasks_.push(t);
      // if past the time threshold, execute immediately
      // and reset the timestamp
      add_task = MaybeGetTask(&task);
    }
    if (add_task) AddTask(task);
  }

  // return the number of tasks that have not been moved as
  //   a result of rate limiting
  virtual int NumPendingTasks() {
    lock_guard<mutex> l(m_);
    return tasks_.size();
  }

 private:
  // wake up every min_ms_between_tasks and process any pending
  // tasks if they exist
  // this function does a slightly unintuitive thing. it allows
  // AT MOST one request per min_ms_between_tasks_. even if you
  // have not had a request for the last 10 seconds, the rate will
  // NOT increase. the rate will stay at most one request per
  // min_ms_between_tasks_
  void ConsumeQueryRate() {
    uint64_t start_ts = 0, end_ts = 0;
    while (run_qps_thread_) {
      uint64_t sleep_time = min_ms_between_tasks_ - (end_ts - start_ts);
      if (sleep_time > 0) {
        this_thread::sleep_for(chrono::milliseconds());
      }
      bool add_task = false;
      T task;
      start_ts = TimestampMs();
      {
        lock_guard<mutex> l(m_);
        add_task = MaybeGetTask(&task);
      }
      if (add_task) AddTask(task);
      end_ts = TimestampMs();
    }
  }

  void AddTask(const T& task) {
    SharedQueue<T>::push(task);
  }

  // checks if it is time to run a new task. fills in the task if
  // a new task is ready to run
  // @warning ASSUMES the mutex is already locked
  // @return true if task is filled with the new task that is ready to run
  bool MaybeGetTask(T* task) {
    bool ready = ReadyForNextTask();
    if (ready) {
      *task = tasks_.back();
      tasks_.pop();
    }
    return ready;
  }

  // checks if another task can be executed and modifies the local state.
  // under the assumption that a new task will be executed if this returns true.
  // @warning ASSUMES the mutex is already locked
  // @warning ASSUMES the task is pushed to tasks_ per executing
  // @modifies last_task_add_timestamp_ms_
  bool ReadyForNextTask() {
    uint64_t timestamp = TimestampMs();
    if (tasks_.size() > 0 &&
        timestamp - last_task_add_timestamp_ms_ >= min_ms_between_tasks_) {
      // reset the state
      last_task_add_timestamp_ms_ = timestamp;
      return true;
    }
    return false;
  }

  uint64_t TimestampMs() {
    return chrono::duration_cast<chrono::milliseconds>(
      chrono::high_resolution_clock::now().time_since_epoch()).count();
  }

  // the minimum ms between each task
  const uint64_t min_ms_between_tasks_;
  // mutex protecting all the mutable data
  mutex m_;
  // the thread that consumes tasks at the appropriate qps
  shared_ptr<thread> consume_qps_thread_;
  // the set of pending tasks
  queue<T> tasks_;
  // the timestamp of the last add
  uint64_t last_task_add_timestamp_ms_;
  // true if the qps thread should continue to run
  bool run_qps_thread_;
};

} // namespace threading
} // namespace util

#endif  // _PUBLIC_UTIL_THREAD_RATE_LIMITED_SHARED_QUEUE_H_
