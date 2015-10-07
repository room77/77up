// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Thread pool class.

#ifndef _PUBLIC_UTIL_THREAD_THREAD_POOL_H_
#define _PUBLIC_UTIL_THREAD_THREAD_POOL_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "base/common.h"
#include "util/factory/factory.h"
#include "util/thread/shared_queue.h"

namespace util {
namespace threading {

typedef std::function<void()> ThreadPoolFunc;

namespace internal {

// Threadpool worker class. Each worker processes callbacks from the queue.
class ThreadPoolWorker {
 public:
  ThreadPoolWorker(int id, SharedQueue<ThreadPoolFunc>* q): id_(id), q_(q) {
    thread_.reset(new std::thread(&ThreadPoolWorker::WorkHorse, this));
  }

  ~ThreadPoolWorker() { Finish(); }

  void Wait() {
    Join();
  }

  void Finish() {
    done_ = true;
    Join();
  }

 private:
  bool Run() {
    vector<ThreadPoolFunc> batch = q_->consume_batch();
    if (batch.empty() && q_->producers_finished()) return false;

    // Run all the function in the batch.
    for (ThreadPoolFunc& f : batch) f();

    return true;
  }

  void Join() {
    VLOG(4) << "Joining Thread: " << id_;
    if (thread_ != NULL) {
      thread_->join();
      thread_.reset(NULL);
      VLOG(4) << "Joined Thread: " << id_;
    }
  }

  void WorkHorse() {
    // Loop on the queue and run callbacks.
    while(!done_ && Run());
    done_ = true;
    VLOG(4) << "Finished Thread: " << id_;
  }

  int id_ = -1;
  SharedQueue<ThreadPoolFunc>* q_;
  unique_ptr<std::thread> thread_;
  volatile bool done_ = false;
};

}  // namespace internal

// Creates a new threadpool of 'n' threads.
class ThreadPool {
 public:
  typedef shared_ptr<SharedQueue<ThreadPoolFunc>> SQ;

  // Creates a thread pool with num_workers with given capacity for the given
  // shared queue.
  ThreadPool(const SQ& q, int num_workers, int capacity = numeric_limits<int>::max())
      : q_(q), capacity_(capacity), size_(0) {
    workers_.reserve(num_workers);
    for (int i = 0; i < num_workers; ++i)
      workers_.push_back(shared_ptr<internal::ThreadPoolWorker>(
          new internal::ThreadPoolWorker(i, q_.get())));
  }

  // Create a new queue with batch size 1 and given capacity.
  ThreadPool(int num_workers, int capacity = numeric_limits<int>::max())
     : ThreadPool(SQ(new SharedQueue<ThreadPoolFunc>(1, capacity)),
                  num_workers, capacity) {}

  ~ThreadPool() { Finish(); }

  int capacity() const { return capacity_; }

  // Returns the number of functions still waiting to be executed or being
  // currently executed.
  int size() const {
    ASSERT(size_ >= 0);
    return size_;
  }

  // The queue size. This does not include the currently running functions but
  // just the functions waiting in the queue.
  int queue_size() const { return q_->size(); }

  // Check if the pool is empty.
  int empty() const { return !size(); }

  // Adds a new func to be executed. Capacity constraints are ignored.
  void Add(ThreadPoolFunc f) {
    // Always increment size before adding to queue.
    ++size_;
    q_->push(std::bind(&ThreadPool::FuncHandler, this, f));
  }

  // Tries to add a new func to be executed. If the threadpool is at capacity
  // the new function is not added. The caller is responsible for calling the
  // func.
  bool TryAdd(ThreadPoolFunc f) {
    ++size_;
    bool res = q_->try_push(std::bind(&ThreadPool::FuncHandler, this, f));
    if (!res) --size_;
    return res;
  }

  // Waits till there are no more functions to run. Note however, that this
  // will wait for all functions to finish (some of which may get added after
  // wait was called).
  void Wait() {
    VLOG(4) << "Waiting, size = " << size();
    std::unique_lock<std::mutex> l(mutex_);
    VLOG(4) << "Wait acquired mutex, size  = " << size();
    cond_.wait(l, std::bind(&ThreadPool::empty, this));
    VLOG(4) << "Finished waiting, size = " << size();
  }

  // Waits with timeout till there are no more functions to run.
  // Note however, that this will wait for all functions to finish
  // (some of which may get added after wait was called).
  // Returns true of the wait ended before timeout and false otherwise.
  bool WaitWithTimeout(int msec = 100) {
    VLOG(4) << "Waiting for " << msec << "ms, size = " << size();
    std::unique_lock<std::mutex> l(mutex_);
    VLOG(4) << "Wait acquired mutex, size  = " << size();
    bool res = cond_.wait_for(l, chrono::milliseconds(msec),
                              std::bind(&ThreadPool::empty, this));
    VLOG(4) << "Finished waiting, size = " << size();
    return res;
  }

 private:
  void Finish() {
    q_->notify_producers_finished();
    for (size_t i = 0; i < workers_.size(); ++i)
      workers_[i]->Wait();
  }

  void FuncHandler(ThreadPoolFunc& f) {
    f();
    --size_;
    if (empty()) {
      std::unique_lock<std::mutex> l(mutex_);
      cond_.notify_all();
    }
  }

  // The shared queue used between threadpool workers.
  SQ q_;

  // Capacity for the queue.
  const int capacity_;
  // List of thread pool workers that run all the functions.
  vector<shared_ptr<internal::ThreadPoolWorker> > workers_;
  // The size of the threadpool. This includes the number of function currently
  // running as well.
  std::atomic_int size_;
  // Mutex for the condition variable.
  mutex mutex_;
  // The condition variable to wait for all functions to finish executing.
  condition_variable cond_;

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool & operator=(const ThreadPool&) = delete;
};

// Thread pool Factory utility class.
class ThreadPoolFactory : public LazyFactory<ThreadPool, string, int, int> {
  typedef LazyFactory<ThreadPool, string, int, int> super;

 public:
  static typename super::mutable_shared_proxy Pool(const string& id,
      int num_workers = 32, int capacity = numeric_limits<int>::max()) {
    return super::make_shared(id, num_workers, capacity,
        [](int num_workers, int capacity) {
            return new ThreadPool(num_workers, capacity);
        });
  }
};

}  // namespace threading
}  // namespace util

#endif  // _PUBLIC_UTIL_THREAD_THREAD_POOL_H_
