// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/templates/container/ra_heap.h"

#include <algorithm>

#include "test/cc/test_main.h"
#include "util/templates/comparator.h"
#include "util/templates/container/test_util.h"

namespace tl {
namespace test {

namespace {

struct tTestItem {
  explicit tTestItem(int val = 0) : val(val) {}
  int val;

  // This is used by RA heap to
  size_t index = -1;
};

}  // namespace

// Test class for RaHeap.
class RaHeapTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    SetupStorageForNItems(10);
  }

  void SetupStorageForNItems(int n = 10) {
    storage_.clear();
    storage_.reserve(n);
    for (int i = 0; i < n; ++i)  storage_.push_back(tTestItem(i));
  }

  void AddNItems(int n = 10) {
    ASSERT_LE(n, storage_.size());
    for (int i = 0; i < n; ++i)  ra_heap_.push(&(storage_[i]));

    // For 10 items, this creates a heap in the form:
    //    ----9----
    //    |       |
    //  --8--   --5--
    //  |   |   |   |
    // -6- -7- -1- -4-
    // | | | | | | | |
    // 0 3 2 x x x x x
  }

  void AddNItemsRandomly(int n = 10) {
    ASSERT_LE(n, storage_.size());
    std::vector<int> iter = GetNShuffledNumbers(n);
    for (size_t i : iter)  ra_heap_.insert(&(storage_[i]));
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  RAHeap<tTestItem, ::util::tl::less_member<tTestItem, int, &tTestItem::val>> ra_heap_;

  vector<tTestItem> storage_;
  vector<int> expected_heap_ = {9, 8, 5, 6, 7, 1, 4, 0, 3, 2};
};

TEST_F(RaHeapTest, Sanity) {
  EXPECT_TRUE(ra_heap_.empty());

  AddNItems();
  EXPECT_FALSE(ra_heap_.empty());
  EXPECT_EQ(10, ra_heap_.size());
  EXPECT_EQ(9, ra_heap_.top()->val);

  for (int i = 0; i < ra_heap_.size(); ++i)
    EXPECT_EQ(expected_heap_[i], ra_heap_.at(i)->val);

  for (int i = ra_heap_.size() - 1; i >= 0; --i) {
    tTestItem* item = ra_heap_.pop();
    EXPECT_EQ(i, item->val);
  }
}

TEST_F(RaHeapTest, RandomInsertionOrder) {
  EXPECT_TRUE(ra_heap_.empty());

  AddNItemsRandomly();
  EXPECT_FALSE(ra_heap_.empty());
  EXPECT_EQ(10, ra_heap_.size());
  EXPECT_EQ(9, ra_heap_.top()->val);

  for (int i = ra_heap_.size() - 1; i >= 0; --i) {
    tTestItem* item = ra_heap_.pop();
    EXPECT_EQ(i, item->val);
  }
}

TEST_F(RaHeapTest, UpdateItemUp) {
  EXPECT_TRUE(ra_heap_.empty());

  AddNItems();
  EXPECT_FALSE(ra_heap_.empty());
  EXPECT_EQ(10, ra_heap_.size());
  EXPECT_EQ(9, ra_heap_.top()->val);

  // Update item 5 to have value 15.
  storage_[4].val = 15;
  EXPECT_EQ(0, ra_heap_.update(&(storage_[4])));
  EXPECT_EQ(15, ra_heap_.top()->val);

  expected_heap_[0] = 15;
  expected_heap_[2] = 9;
  expected_heap_[6] = 5;
  for (int i = 0; i < ra_heap_.size(); ++i)
    EXPECT_EQ(expected_heap_[i], ra_heap_.at(i)->val);
}

TEST_F(RaHeapTest, UpdateItemDown) {
  EXPECT_TRUE(ra_heap_.empty());

  AddNItems();
  EXPECT_FALSE(ra_heap_.empty());
  EXPECT_EQ(10, ra_heap_.size());
  EXPECT_EQ(9, ra_heap_.top()->val);
  EXPECT_EQ(9, storage_[2].index);
  EXPECT_EQ(1, storage_[8].index);

  // Update item 8 to have value 2.
  storage_[8].val = 2;
  EXPECT_EQ(4, ra_heap_.update(&(storage_[8])));
  EXPECT_EQ(9, ra_heap_.top()->val);

  expected_heap_[1] = 7;
  expected_heap_[4] = 2;
  for (int i = 0; i < ra_heap_.size(); ++i)
    EXPECT_EQ(expected_heap_[i], ra_heap_.at(i)->val);

  EXPECT_EQ(4, storage_[8].index);
  EXPECT_EQ(9, storage_[2].index);
}

TEST_F(RaHeapTest, EraseItem) {
  EXPECT_TRUE(ra_heap_.empty());

  AddNItems();
  EXPECT_FALSE(ra_heap_.empty());
  EXPECT_EQ(10, ra_heap_.size());
  EXPECT_EQ(9, ra_heap_.top()->val);
  EXPECT_EQ(1, storage_[8].index);

  // Erase the item at 8.
  ra_heap_.erase(&(storage_[8]));
  EXPECT_EQ(9, ra_heap_.top()->val);
  EXPECT_EQ(9, ra_heap_.size());

  expected_heap_[1] = 7;
  expected_heap_[4] = 2;
  for (int i = 0; i < ra_heap_.size(); ++i)
    EXPECT_EQ(expected_heap_[i], ra_heap_.at(i)->val);
}

TEST_F(RaHeapTest, ALotOfItems) {
  SetupStorageForNItems(1000);
  AddNItemsRandomly(1000);
  EXPECT_FALSE(ra_heap_.empty());
  EXPECT_EQ(1000, ra_heap_.size());
  EXPECT_EQ(999, ra_heap_.top()->val);

  for (int i = ra_heap_.size() - 1; i >= 0; --i)
    EXPECT_EQ(i, ra_heap_.pop()->val);
}

}  // namespace test
}  // namespace tl
