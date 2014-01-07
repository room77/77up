// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/templates/container/best_n.h"

#include <algorithm>
#include <memory>

#include "test/cc/unit_test.h"
#include "util/templates/container/test_util.h"

namespace tl {
namespace test {

// Test class for BestN.
class BestNTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    best_n_.reset(new BestN<int>(10));
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  void AddNItems(int n = 10) {
    for (int i = 0; i < n; ++i)
      EXPECT_EQ(kBestNInsertStatusAddedNew, best_n_->insert("key:" + to_string(i), i));
  }

  void AddNItemsRandomly(int n = 10, bool check_status = false) {
    std::vector<int> iter = GetNShuffledNumbers(n);

    for (int i : iter) {
      BestNInsertStatus status = best_n_->insert("key:" + to_string(i), i);
      if (check_status)
        EXPECT_EQ(kBestNInsertStatusAddedNew, status);
    }
  }

  unique_ptr<BestN<int>> best_n_;
};

TEST_F(BestNTest, Sanity) {
  EXPECT_TRUE(best_n_->empty());

  AddNItemsRandomly();
  EXPECT_FALSE(best_n_->empty());
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  vector<int> res = best_n_->GetBestN();
  EXPECT_EQ(res.size(), best_n_->size());
  for (int i = 0; i < res.size(); ++i)
    EXPECT_EQ(i, res[i]);
}

TEST_F(BestNTest, UpdateItem) {
  EXPECT_TRUE(best_n_->empty());

  AddNItemsRandomly();
  EXPECT_FALSE(best_n_->empty());
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  EXPECT_EQ(kBestNInsertStatusDidNotAddExistingBetter,
            best_n_->insert("key:" + to_string(4), 5));

  EXPECT_EQ(kBestNInsertStatusDidNotAddExistingBetter,
            best_n_->insert("key:" + to_string(4), 4));

  EXPECT_EQ(kBestNInsertStatusAddedExistingWorse,
            best_n_->insert("key:" + to_string(4), 2));

  vector<int> res = best_n_->GetBestN();
  EXPECT_EQ(res.size(), best_n_->size());

  vector<int> expected = {0, 1, 2, 2, 3, 5, 6, 7, 8, 9};
  for (int i = 0; i < res.size(); ++i)
    EXPECT_EQ(expected[i], res[i]);
}

TEST_F(BestNTest, AddItemOverCapacity) {
  EXPECT_TRUE(best_n_->empty());

  AddNItemsRandomly();
  EXPECT_FALSE(best_n_->empty());
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  EXPECT_EQ(kBestNInsertStatusDidNotAddNewWorse,
            best_n_->insert("key:" + to_string(10), 10));
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  EXPECT_EQ(kBestNInsertStatusDidNotAddNewWorse,
            best_n_->insert("key:" + to_string(10), 9));
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  EXPECT_EQ(kBestNInsertStatusAddedNew,
            best_n_->insert("key:" + to_string(10), 7));
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(8, best_n_->worst());

  vector<int> res = best_n_->GetBestN();
  EXPECT_EQ(res.size(), best_n_->size());

  vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 7, 8};
  for (int i = 0; i < res.size(); ++i)
    EXPECT_EQ(expected[i], res[i]);
}

TEST_F(BestNTest, ALotOfItems) {
  EXPECT_TRUE(best_n_->empty());

  AddNItemsRandomly(1000);
  EXPECT_FALSE(best_n_->empty());
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  vector<int> res = best_n_->GetBestN();
  EXPECT_EQ(res.size(), best_n_->size());
  for (int i = 0; i < res.size(); ++i)
    EXPECT_EQ(i, res[i]);
}

TEST_F(BestNTest, RebuildHeap) {
  EXPECT_TRUE(best_n_->empty());

  AddNItemsRandomly();
  EXPECT_FALSE(best_n_->empty());
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  vector<int> res = best_n_->GetBestN();
  EXPECT_EQ(res.size(), best_n_->size());
  for (int i = 0; i < res.size(); ++i)
    EXPECT_EQ(i, res[i]);

  // Call this again.
  res = best_n_->GetBestN();
  EXPECT_EQ(res.size(), best_n_->size());
  for (int i = 0; i < res.size(); ++i)
    EXPECT_EQ(i, res[i]);
}

TEST_F(BestNTest, SerializeBinary) {
  EXPECT_TRUE(best_n_->empty());

  AddNItemsRandomly();
  EXPECT_FALSE(best_n_->empty());
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  vector<int> res = best_n_->GetBestN();
  EXPECT_EQ(::serial::Serializer::ToBinary(res), ::serial::Serializer::ToBinary(*best_n_));
}

TEST_F(BestNTest, SerializeJSON) {
  EXPECT_TRUE(best_n_->empty());

  AddNItemsRandomly();
  EXPECT_FALSE(best_n_->empty());
  EXPECT_EQ(10, best_n_->size());
  EXPECT_EQ(9, best_n_->worst());

  vector<int> res = best_n_->GetBestN();
  EXPECT_EQ(::serial::Serializer::ToJSON(res), ::serial::Serializer::ToJSON(*best_n_));
}

}  // namespace test
}  // namespace tl
