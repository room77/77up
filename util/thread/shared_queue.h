// Copyright 2012 Room77, Inc.
// Author: Nicholas Edelman

//
// Simple threadsafe queue that protects STL queue methods with a mutex.
//

#ifndef _PUBLIC_UTIL_THREAD_SHARED_QUEUE_H_
#define _PUBLIC_UTIL_THREAD_SHARED_QUEUE_H_

#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "base/common.h"
#include "util/factory/factory.h"

namespace util {
namespace threading {

template<typename T>
class SharedQueue {
 public:
  // @param batch_size - the number of elements in the queue before
  //        the producer is notified
  // @param capacity - the max number of elements in the queue after which
  //        try_push starts returning false.
  // @param notify_timeout - the timeout in seconds to notify the producer.
  //        this notification will happen even if the batch_size is NOT reached.
  //        if timeout < 0, it is not activate
  SharedQueue(int batch_size = 1, int capacity = numeric_limits<int>::max(),
              int notify_timeout_sec = -1) :
      batch_size_(batch_size),
      capacity_(capacity),
      notify_timeout_(notify_timeout_sec),
      producers_finished_(false),
      last_flush_ts_(std::chrono::high_resolution_clock::now()) {
    if (notify_timeout_ > 0) {
      run_timeout_thread_ = true;
      timeout_notify_thread_.reset(
          new thread(&SharedQueue<T>::timeout_consume, this));
    }
  }

  virtual ~SharedQueue() {
    notify_producers_finished();
    if (timeout_notify_thread_ != NULL) {
      run_timeout_thread_ = false;
      timeout_notify_thread_->join();
    }
  }

  int batch_size() const { return batch_size_; }
  int capacity() const { return capacity_; }

  // Note that these values are approximate. They may change after the function
  // returns. Any critical code should not depend on these values.
  bool empty() { lock_guard<mutex> l(mutex_); return q_.empty(); }
  size_t size() { lock_guard<mutex> l(mutex_); return q_.size(); }

  bool producers_finished() {
    lock_guard<mutex> l(mutex_);
    return producers_finished_;
  }

  // This will always add to the queue regardless of the capacity.
  virtual void push(const T& t) {
    lock_guard<mutex> l(mutex_);
    unlocked_push(t);
  }

  bool try_push(const T& t) {
    lock_guard<mutex> l(mutex_);
    if (q_.size() >= capacity()) return false;

    unlocked_push(t);
    return true;
  }

  T pop() {
    lock_guard<mutex> l(mutex_);
    T data = q_.front();
    q_.pop();
    return data;
  }

  // blocking wait on conditional variable to consume the
  // next batch of results from the queue.
  vector<T> consume_batch() {
    vector<T> batch;
    // unique lock needed for cond wait
    unique_lock<mutex> l(mutex_);
    if (q_.size() < batch_size() && !producers_finished_)
      cond_.wait(l);

    // Get the batch from the queue.
    int size = std::min<int>(batch_size(), q_.size());
    batch.reserve(size);
    for (int i = 0; i < size; ++i) {
      batch.push_back(q_.front());
      q_.pop();
    }
    // update the last flush timestamp
    last_flush_ts_ = std::chrono::high_resolution_clock::now();
    return batch;
  }

  // clear out the queue and push into batch.
  vector<T> flush() {
    vector<T> batch;
    lock_guard<mutex> l(mutex_);
    // flush the queue
    batch.reserve(q_.size());
    for (int i = 0; i < q_.size(); ++i) {
      batch.push_back(q_.front());
      q_.pop();
    }
    // update the last flush timestamp
    last_flush_ts_ = std::chrono::high_resolution_clock::now();
    return batch;
  }

  // This function allows a queue manager to wake up all consumers once all
  // producers are finished.
  void notify_producers_finished() {
    unique_lock<mutex> l(mutex_);
    producers_finished_ = true;
    cond_.notify_all();
  }

 private:
  // Add to the queue. The caller must ensure that 'mutex_' is already held.
  void unlocked_push(const T& t) {
    q_.push(t);
    if (q_.size() >= batch_size())
      cond_.notify_one();
  }

  // force the batch to be consumed if it is non-empty by
  // notifying the condition variable
  void timeout_consume() {
    while (run_timeout_thread_) {
      this_thread::sleep_for(chrono::seconds(notify_timeout_));
      unique_lock<mutex> l(mutex_);
      if (producers_finished_) cond_.notify_all();
      else if (q_.size() > 0 && std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::high_resolution_clock::now() - last_flush_ts_).count() >=
          notify_timeout_) cond_.notify_one();
    }
  }

  const int batch_size_;
  const int capacity_;
  const int notify_timeout_;
  // Thread that notifies a consumer after timeout.
  unique_ptr<thread> timeout_notify_thread_;
  volatile bool run_timeout_thread_;
  volatile bool producers_finished_;
  // the timestamp of the last flush
  std::chrono::time_point<std::chrono::high_resolution_clock> last_flush_ts_;
  queue<T> q_;
  mutex mutex_;
  condition_variable cond_;
};

template<typename T>
class SharedQueueFactory :
    public LazyFactory<SharedQueue<T>, string, int, int, int> {
  typedef LazyFactory<SharedQueue<T>, string, int, int, int> super;

 public:
  static typename super::shared_proxy Queue(const string& id, int batch_size = 1,
      int capacity = numeric_limits<int>::max(), int notify_timeout = -1) {
    return super::make_shared(id, batch_size, capacity, notify_timeout,
        [](int batch_size, int capacity, int notify_timeout) {
      return new SharedQueue<T>(batch_size, capacity, notify_timeout);
    });
  }
};

}  // namespace threading
}  // namespace util

#endif  // _PUBLIC_UTIL_THREAD_SHARED_QUEUE_H_
