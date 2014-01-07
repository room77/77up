// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test file for thread_pool.

#include <functional>
#include <thread>

#include "util/thread/counters.h"

#include "test/cc/unit_test.h"

namespace util {
namespace threading {
namespace test {

TEST(Notification, Sanity) {
  { Notification dummy; }

  bool set = false;
  Notification started("started"), wait("wait"), notify("notify");
  thread t([&set, &started, &wait, &notify](){ started.Notify();
      EXPECT_FALSE(set); wait.Wait(); set = true; notify.Notify(); });
  EXPECT_FALSE(set);
  started.Wait();  // Wait for thread to start.
  EXPECT_FALSE(set);
  wait.Notify();  // Notify the thread to modify data.
  notify.Wait();  // Wait for the thread to modify data.
  EXPECT_TRUE(set);
  t.join();
}

TEST(Notification, Scoped) {
  bool set = false;
  Notification started("started"), wait("wait"), notify("notify");
  thread t([&set, &started, &wait, &notify](){ started.Notify();
      EXPECT_FALSE(set); wait.Wait(); set = true; ScopedNNotify n(&notify);});
  EXPECT_FALSE(set);
  started.Wait();  // Wait for thread to start.
  EXPECT_FALSE(set);
  wait.Notify();  // Notify the thread to modify data.
  notify.Wait();  // Wait for the thread to modify data.
  EXPECT_TRUE(set);
  t.join();
}

TEST(Notification, WaitWithTimeout) {
  Notification started("started"), wait("wait"), n1("n1"), n2("n2");
  thread t([&n1, &n2, &started, &wait](){ started.Notify();
      wait.Wait(); n1.Notify(); this_thread::sleep_for(std::chrono::seconds(1));
      n2.Notify(); });
  started.Wait();  // Wait for thread to start.

  wait.Notify();  // Notify the thread to notify counter.

  EXPECT_TRUE(n1.WaitWithTimeout(20));

  EXPECT_FALSE(n2.WaitWithTimeout(20));

  n2.Wait();  // Wait for the thread to notify counter.
  t.join();
}

TEST(Counters, Decrement) {
  { Counter dummy; }

  Counter c(1);
  Notification started("started"), wait("wait");
  thread t([&c, &started, &wait](){ started.Notify(); EXPECT_EQ(1, c.count());
      wait.Wait(); c.Notify(); });
  started.Wait();  // Wait for thread to start.
  EXPECT_EQ(1, c.count());
  wait.Notify();  // Notify the thread to notify counter.
  c.Wait();  // Wait for the thread to notify counter.
  EXPECT_EQ(0, c.count());
  t.join();
}

TEST(Counters, ScopedDecrement) {
  { Counter dummy; }

  Counter c(1);
  Notification started("started"), wait("wait");
  thread t([&c, &started, &wait](){started.Notify(); EXPECT_EQ(1, c.count());
      wait.Wait(); ScopedCNotify n(&c); });
  started.Wait();  // Wait for thread to start.
  EXPECT_EQ(1, c.count());
  wait.Notify();  // Notify the thread to notify counter.
  c.Wait();  // Wait for the thread to notify counter.
  EXPECT_EQ(0, c.count());
  t.join();
}

TEST(Counters, Increment) {
  Counter c(-1);
  Notification started("started"), wait("wait");
  thread t([&c, &started, &wait](){ started.Notify(); EXPECT_EQ(-1, c.count());
      wait.Wait(); c.Increment(); });
  started.Wait();  // Wait for thread to start.
  EXPECT_EQ(-1, c.count());
  wait.Notify();  // Notify the thread to notify counter.
  c.Wait();  // Wait for the thread to notify counter.
  EXPECT_EQ(0, c.count());
  t.join();
}

TEST(Counters, WaitWithTimeout) {
  Counter c1(1), c2(1);
  Notification started("started"), wait("wait");
  thread t([&c1, &c2, &started, &wait](){ started.Notify();
      wait.Wait(); c1.Notify(); this_thread::sleep_for(std::chrono::seconds(1));
      c2.Notify(); });
  started.Wait();  // Wait for thread to start.
  EXPECT_EQ(1, c1.count());
  EXPECT_EQ(1, c2.count());

  wait.Notify();  // Notify the thread to notify counter.

  EXPECT_TRUE(c1.WaitWithTimeout(20));
  EXPECT_EQ(0, c1.count());

  EXPECT_FALSE(c2.WaitWithTimeout(20));
  EXPECT_EQ(1, c2.count());

  c2.Wait();  // Wait for the thread to notify counter.
  EXPECT_EQ(0, c2.count());
  t.join();
}

}  // namepace test
}  // namespace threading
}  // namespace util
