// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_UTIL_THREAD_COUNTERS_H_
#define _PUBLIC_UTIL_THREAD_COUNTERS_H_

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "base/common.h"

namespace util {
namespace threading {

// Notification mechanism used by two threads to synchronize multiple threads.
// Multiple threads can wait for the same notfication.
class Notification {
 public:
  Notification(const string& name = "") : name_(name) {}
  ~Notification() { Notify(); }

  void Notify() {
    lock_guard<mutex> l(mutex_);
    notified_ = true;
    VLOG(5) << name_ << ":" <<  std::this_thread::get_id() << " notifying.";
    cond_.notify_all();
  }

  void Wait() {
    std::unique_lock<std::mutex> l(mutex_);
    if (notified_) {
      VLOG(5) << name_ << ":" <<  std::this_thread::get_id()
             << "Already notified. No need to wait.";
      return;
    }
    cond_.wait(l, [this]() -> bool { return notified_; });
    VLOG(5) << name_ << ":" <<  std::this_thread::get_id() << " done waiting";
  }

  // Wait with a timeout.
  // Returns true of the wait ended before timeout and false otherwise.
  bool WaitWithTimeout(int msec) {
    std::unique_lock<std::mutex> l(mutex_);
    if (notified_) {
      VLOG(5) << name_ << ":" <<  std::this_thread::get_id()
             << "Already notified. No need to wait.";
      return true;
    }

    bool res = cond_.wait_for(l, chrono::milliseconds(msec),
                              [this]() -> bool { return notified_; });
    VLOG(5) << name_ << ":" <<  std::this_thread::get_id() << " done waiting";
    return res;
  }

  const string& name() const {return name_;}

 private:
  // Set to true if notify has already been called.
  bool notified_ = false;
  // Name for the notification.
  const string name_;
  // Mutex for the condition variable.
  mutex mutex_;
  // The condition variable to wait for the notification.
  condition_variable cond_;

  Notification(const Notification&) = delete;
  Notification & operator=(const Notification&) = delete;
};

// Allows multiple threads to wait for the counter to become 0.
class Counter {
 public:
  explicit Counter(int count = 0, const string& name = "")
      : count_(count), name_(name) { }
  ~Counter() { Reset(); }

  void ChangeBy(int val) {
    lock_guard<mutex> l(mutex_);
    count_ += val;
    if (count_ == 0) {
      VLOG(5) << name_ << ":"  <<  std::this_thread::get_id() << " notifying.";

      cond_.notify_all();
    }
  }

  void Increment() { ChangeBy(1); }
  void Decrement() { ChangeBy(-1); }
  void Notify() { Decrement(); }

  void Reset() {
    lock_guard<mutex> l(mutex_);
    count_ = 0;
    cond_.notify_all();
  }

  void Wait() {
    std::unique_lock<std::mutex> l(mutex_);
    if (count_ == 0) {
      VLOG(5) << name_ << ":" <<  std::this_thread::get_id()
             << " Already at 0. No need to wait.";
      return;
    }
    cond_.wait(l, [this]() -> bool { return count_ == 0; });
    VLOG(5) << name_ << ":" <<  std::this_thread::get_id() << " done waiting";
  }

  // Wait with a timeout.
  // Returns true of the wait ended before timeout and false otherwise.
  bool WaitWithTimeout(int msec) {
    std::unique_lock<std::mutex> l(mutex_);
    if (count_ == 0) {
      VLOG(5) << name_ << ":" <<  std::this_thread::get_id()
             << " Already at 0. No need to wait.";
      return true;
    }

    bool res = cond_.wait_for(l, chrono::milliseconds(msec),
                              [this]() -> bool { return count_ == 0; });
    VLOG(5) << name_ << ":" <<  std::this_thread::get_id() << " done waiting";

    return res;
  }

  int count() { lock_guard<mutex> l(mutex_); return count_; }

  const string& name() const {return name_;}

 private:
  // Set to true if notify has already been called.
  int count_ = 0;

  // The name for the counter.
  const string name_;
  // Mutex for the condition variable.
  mutex mutex_;
  // The condition variable to wait for the notification.
  condition_variable cond_;

  Counter(const Counter&) = delete;
  Counter & operator=(const Counter&) = delete;
};

template <class T>
class ScopedNotify {
 public:
  ScopedNotify() {}
  ScopedNotify(T* notify) : notify_(notify) {}
  ~ScopedNotify() {if (notify_) notify_->Notify(); }

  void reset(T* notify) { if (notify_) notify_->Notify(); notify_ = notify; }
  T* release() { T* res = notify_; notify_ = nullptr; return res; }

 private:
  T* notify_ = nullptr;

  ScopedNotify(const ScopedNotify&) = delete;
  ScopedNotify & operator=(const ScopedNotify&) = delete;
};

typedef ScopedNotify<Notification> ScopedNNotify;
typedef ScopedNotify<Counter> ScopedCNotify;

}  // namespace threading
}  // namespace util


#endif  // _PUBLIC_UTIL_THREAD_COUNTERS_H_
