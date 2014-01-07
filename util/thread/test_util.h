// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_THREAD_TEST_UTIL_H_
#define _PUBLIC_UTIL_THREAD_TEST_UTIL_H_

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "base/defs.h"
#include "base/logging.h"

#include "util/thread/counters.h"
#include "util/thread/shared_queue.h"
#include "util/thread/thread_pool.h"

namespace util {
namespace threading {

namespace test {

// Returns a new item for basic types in incremental order.
template<typename T>
void get_new_item(T* t) {
  static int item = 0;
  static mutex m;
  lock_guard<mutex> l(m);
  *t = ++item;
}

// Returns a new string item in incremental order.
template<>
inline void get_new_item(string* t) {
  static int item = 0;
  static mutex m;
  int temp = 0;
  { lock_guard<mutex> l(m);
    temp = ++item;
  }
  *t = strutil::ToString(temp);
}

// Base class for producers.
// Creates an async thread to produce if required.
class ProducerBase {
 public:
  ProducerBase(int id, bool async = false) : id_(id), async_(async) {
    if (async_)
      thread_.reset(new thread(&ProducerBase::ProduceAsync, this));
  }

  void BeginAsyncProduction() {
    begin_async_.Notify();
  }

  virtual ~ProducerBase() { if (async_) Finish(); }

  void Finish() {
    if (async_ && !done_) {
      done_ = true;
      thread_->join();
    }
  }

  // Produces a new item.
  virtual void Produce() = 0;

 private:
  void ProduceAsync() {
    begin_async_.Wait();
    while(!done_) Produce();
    VLOG(3) << "PD: " << id_;
  }

  const int id_;
  const bool async_;
  unique_ptr<thread> thread_;
  volatile bool done_ = false;
  Notification begin_async_;
};

// Creates new items for the fiven type and adds them to the queue.
template <typename T, void (*F)(T*)=get_new_item<T> >
class SharedQProducer : public ProducerBase {
  typedef ProducerBase super;
 public:
  SharedQProducer(int id, SharedQueue<T>* q, bool async = false, bool begin_async = true)
      : super(id, async), q_(q) {
    // Derived classes should set begin_async to false. And call BeginAsyncProduction after the
    // final object has been fully constructed, otherwise the Produce call may lead to
    // unexpected behavior.
    if (async && begin_async) BeginAsyncProduction();
  }

  void Produce() {
    T t;
    F(&t);
    produced_.push_back(t);
    q_->push(t);
  }

  const vector<T>& produced() const { return produced_; }

 private:
  SharedQueue<T>* q_;
  vector<T> produced_;
};

class ThreadPoolProducer : public ProducerBase {
  typedef ProducerBase super;
 public:
  ThreadPoolProducer(int id, ThreadPool* pool, const ThreadPoolFunc& func,
                     bool async = false, bool begin_async = true)
     : super(id, async), pool_(pool), func_(func) {
    // Derived classes should set begin_async to false. And call BeginAsyncProduction after the
    // final object has been fully constructed, otherwise the Produce call may lead to
    // unexpected behavior.
    if (async && begin_async) BeginAsyncProduction();
  }

  virtual void Produce() {
    ++produced_;
    pool_->Add(func_);
  }

  int produced() const { return produced_; }

 private:
  ThreadPool* pool_ = nullptr;
  ThreadPoolFunc func_;
  int produced_ = 0;
};

}  // namepace test
}  // namespace threading
}  // namespace util


#endif  // _PUBLIC_UTIL_THREAD_TEST_UTIL_H_
