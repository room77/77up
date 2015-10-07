// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Some utility consumers for the shared queue.

#ifndef _PUBLIC_UTIL_THREAD_SHARED_QUEUE_CONSUMERS_H_
#define _PUBLIC_UTIL_THREAD_SHARED_QUEUE_CONSUMERS_H_

#include <mutex>
#include <thread>
#include <vector>

#include "base/common.h"
#include "util/thread/shared_queue.h"

namespace util {
namespace threading {

template<typename Container>
class StandardConsumer {
 public:
  typedef typename Container::value_type T;
  StandardConsumer(SharedQueue<T>* q): q_(q) {}
  virtual ~StandardConsumer() {}

  virtual bool Consume() {
    vector<T> batch = q_->consume_batch();
    bool res = true;
    if (batch.size()) {
      lock_guard<mutex> l(mutex_);
      consumed_.insert(consumed_.end(), batch.begin(), batch.end());
    } else {
      if (q_->producers_finished()) res = false;
    }
    return res;
  }

  // Extracts the consumed data from the consumer.
  void ExtractConsumed(Container* res) {
    res->clear();
    lock_guard<mutex> l(mutex_);
    res->swap(consumed_);
  }

  // This returns an unsafe reference to the consumed data.
  // Make sure to use it appropriately.
  const Container& Consumed() const { return consumed_; }

 protected:
  SharedQueue<T>* q_;
  Container consumed_;
  mutex mutex_;
};


template<typename Container>
class AsyncConsumer : public StandardConsumer<Container> {
  typedef StandardConsumer<Container> super;

 public:
  AsyncConsumer(int id, SharedQueue<typename super::T>* q): super(q), id_(id)  {
    thread_.reset(new thread(&AsyncConsumer::ConsumeInternal, this));
  }

  ~AsyncConsumer() { Finish(); }

  // Wait for the consumer to finish.
  void Wait() { JoinThread(); }

  // Force the consumer to finish.
  void Finish() {
    done_ = true;
    JoinThread();
  }

 protected:
  void JoinThread() {
    lock_guard<mutex> l(thread_mutex_);
    if (thread_ != NULL) {
      thread_->join();
      VLOG(3) << "Joined: " << id_;
      thread_.reset(NULL);
    }
  }

  void ConsumeInternal() {
    while(!done_ && this->Consume());
    VLOG(3) << "Done: " << id_;
    done_ = true;
  }

  const int id_;
  unique_ptr<thread> thread_;
  volatile bool done_ = false;
  mutex thread_mutex_;
};

}  // namespace threading
}  // namespace util


#endif  // _PUBLIC_UTIL_THREAD_SHARED_QUEUE_CONSUMERS_H_
