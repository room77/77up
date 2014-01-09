// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test file for shared_queue.

#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "test/cc/test_main.h"
#include "util/string/strutil.h"
#include "util/thread/shared_queue.h"
#include "util/thread/shared_queue_consumers.h"
#include "util/thread/test_util.h"

namespace util {
namespace threading {
namespace test {

TEST(SharedQueue, Sanity) {
  SharedQueue<int> q;
  q.push(1);
  int val = q.pop();
  EXPECT_EQ(1, val);
}

template<typename T>
class SharedQueueTest : public testing::Test {
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

typedef testing::Types<int, string> AllTypes;
TYPED_TEST_CASE(SharedQueueTest, AllTypes);

TYPED_TEST(SharedQueueTest, SimpleProducerConsumer) {
  SharedQueue<TypeParam> q;

  // Generate an asynch consumer.
  AsyncConsumer<vector<TypeParam> > c(0, &q);

  SharedQProducer<TypeParam> p(0, &q, false);
  for (int i = 0; i < 5; ++i)
    p.Produce();

  q.notify_producers_finished();

  c.Wait();
  EXPECT_EQ(0, q.size());
  EXPECT_TRUE(std::equal(c.Consumed().begin(), c.Consumed().end(),
                         p.produced().begin()));
}

TYPED_TEST(SharedQueueTest, MultipleProducerConsumer) {
  SharedQueue<TypeParam> q;

  vector<shared_ptr<SharedQProducer<TypeParam> > > producers;
  vector<shared_ptr<AsyncConsumer<vector<TypeParam> > > > consumers;

  for (int i = 0; i < 10; ++i) {
    consumers.push_back(shared_ptr<AsyncConsumer<vector<TypeParam> > >(
        new AsyncConsumer<vector<TypeParam> >(i, &q)));
    producers.push_back(shared_ptr<SharedQProducer<TypeParam> >(
        new SharedQProducer<TypeParam>(i, &q, true)));
  }

  this_thread::sleep_for(chrono::milliseconds(100));

  for (int i = 0; i < producers.size(); ++i)
    producers[i]->Finish();

  q.notify_producers_finished();

  vector<TypeParam> produced;
  for (int i = 0; i < producers.size(); ++i)
    produced.insert(produced.end(), producers[i]->produced().begin(),
                    producers[i]->produced().end());

  for (int i = 0; i < consumers.size(); ++i)
    consumers[i]->Wait();

  set<TypeParam> consumed;
  for (int i = 0; i < consumers.size(); ++i)
    consumed.insert(consumers[i]->Consumed().begin(),
                    consumers[i]->Consumed().end());

  EXPECT_EQ(0, q.size());
  EXPECT_EQ(produced.size(), consumed.size());
}

TYPED_TEST(SharedQueueTest, FactoryQueueSimpleProducerConsumer) {
  typename SharedQueueFactory<TypeParam>::mutable_shared_proxy proxy =
      SharedQueueFactory<TypeParam>::Queue("test");
  ASSERT_NOTNULL(proxy);

  // Generate an asynch consumer.
  AsyncConsumer<vector<TypeParam> > c(0, proxy.get());

  SharedQProducer<TypeParam> p(0, proxy.get(), false);
  for (int i = 0; i < 5; ++i)
    p.Produce();

  proxy->notify_producers_finished();

  c.Wait();
  EXPECT_EQ(0, proxy->size());
  EXPECT_TRUE(std::equal(c.Consumed().begin(), c.Consumed().end(),
                         p.produced().begin()));
}

}  // namepace test
}  // namespace threading
}  // namespace util
