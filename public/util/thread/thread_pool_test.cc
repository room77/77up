// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test file for thread_pool.

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "test/cc/test_main.h"
#include "util/string/strutil.h"
#include "util/thread/counters.h"
#include "util/thread/thread_pool.h"
#include "util/thread/test_util.h"

namespace util {
namespace threading {
namespace test {

namespace {

class SimpleCounter {
 public:
  SimpleCounter() : count_(0) {}

  void Callback() {
    // this_thread::sleep_for(chrono::microseconds(100));
    ++count_;
  }
  int count() const { return count_; }

 private:
  std::atomic_int count_;
};

}  // namespace

TEST(ThreadPool, Sanity) {
  ThreadPool pool(32);
  SimpleCounter c;
  std::function<void()> func = std::bind(&SimpleCounter::Callback, &c);
  pool.Add(func);
  pool.Wait();

  EXPECT_EQ(1, c.count());
}

TEST(ThreadPool, SimpleProducer) {
  ThreadPool pool(32);
  SimpleCounter c;
  std::function<void()> func = std::bind(&SimpleCounter::Callback, &c);

  ThreadPoolProducer p(0, &pool, func, false);
  for (int i = 0; i < 5; ++i)
    p.Produce();

  pool.Wait();
  EXPECT_EQ(p.produced(), c.count());
}

TEST(ThreadPool, MultipleProducers) {
  ThreadPool pool(128);
  SimpleCounter c;
  std::function<void()> func = std::bind(&SimpleCounter::Callback, &c);

  vector<shared_ptr<ThreadPoolProducer>> producers;
  producers.reserve(256);
  for (int i = 0; i < 256; ++i) {
    producers.push_back(shared_ptr<ThreadPoolProducer>(
        new ThreadPoolProducer(i, &pool, func, true)));
  }

  // Wait a while to let them produce some items.
  this_thread::sleep_for(chrono::milliseconds(10));

  for (int i = 0; i < producers.size(); ++i)
    producers[i]->Finish();

  pool.Wait();

  int produced = 0;
  for (int i = 0; i < producers.size(); ++i)
    produced += producers[i]->produced();

  EXPECT_EQ(produced, c.count());
}

TEST(ThreadPool, WaitWithTimeout) {
  ThreadPool pool(2);
  Notification start("start"), wait("wait"), notify("notify");
  pool.Add([&start, &wait, &notify](){
    start.Notify(); wait.Wait(); notify.Notify(); });

  start.Wait();
  EXPECT_FALSE(pool.WaitWithTimeout(10));
  wait.Notify();
  notify.Wait();
  EXPECT_TRUE(pool.WaitWithTimeout(10));
}

}  // namepace test
}  // namespace threading
}  // namespace util
