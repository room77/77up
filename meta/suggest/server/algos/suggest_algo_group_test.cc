// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/suggest/server/algos/suggest_algo_group.h"

#include "meta/suggest/server/algos/suggest_algo_mock.h"
#include "meta/suggest/server/test_util/suggest_server_test_util.h"
#include "test/cc/test_main.h"
#include "util/thread/thread_pool.h"

namespace suggest {
namespace algo {
namespace test {

using ::suggest::test::MakeMockSuggestRequest;
using ::suggest::test::MakeMockSuggestResponse;
using ::testing::_;

// Test class for MyTest.
class SuggestAlgoGroupTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    string params =
        "{"
          "\"id\": \"suggest_algo_def_test_group\","
          "\"algo_params\": ["
            "{"
              "\"id\": \"suggest_algo_mock1\","
              "\"weight\": 100,"
              "\"required\": true"
            "}, {"
              "\"id\": \"suggest_algo_mock2\","
              "\"weight\": 20,"
              "\"op\": \"+\","
              "\"required\": false"
            "}"
          "]"
        "}";

    // Register suggest algo primary group.
    SuggestAlgo::bind("suggest_algo_test_group", params,
        InitializeConfigureConstructor<SuggestAlgoGroup, string>());
    RegisterNewMockSuggestAlgo("suggest_algo_mock1");
    RegisterNewMockSuggestAlgo("suggest_algo_mock2");
  }

  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    pool_.reset(new ::util::threading::ThreadPool(4));
    context_.reset(new SuggestAlgoContext());

    ResetRequestAndResponse();
  }

  virtual void ResetRequestAndResponse() {
    request_ = MakeMockSuggestRequest("test");
    response_.reset(new SuggestResponse());
  }

  // Use the Pool.
  virtual void UseThreadPool() {
    context_->pool = pool_.get();
  }

  // Sets up default values for all values.
  virtual void SetUpDefaults() {
    algo_group_ = SuggestAlgo::make_shared("suggest_algo_test_group");
    ASSERT_NOTNULL(algo_group_);

    mock_algo_proxy_1_ = SuggestAlgo::make_shared("suggest_algo_mock1");
    mock_algo_proxy_2_ = SuggestAlgo::make_shared("suggest_algo_mock2");
    mock_algo1_ = dynamic_cast<MockSuggestAlgo*>(mock_algo_proxy_1_.get());
    mock_algo2_ = dynamic_cast<MockSuggestAlgo*>(mock_algo_proxy_2_.get());
    ASSERT_NOTNULL(mock_algo1_);
    ASSERT_NOTNULL(mock_algo2_);
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  SuggestAlgo::shared_proxy algo_group_;
  SuggestAlgo::mutable_shared_proxy mock_algo_proxy_1_;
  SuggestAlgo::mutable_shared_proxy mock_algo_proxy_2_;
  MockSuggestAlgo* mock_algo1_ = nullptr;
  MockSuggestAlgo* mock_algo2_ = nullptr;

  SuggestRequest request_;
  shared_ptr<SuggestResponse> response_;
  unique_ptr< ::util::threading::ThreadPool> pool_;
  shared_ptr<SuggestAlgoContext> context_;
};

TEST_F(SuggestAlgoGroupTest, MakeShared) {
  algo_group_ = SuggestAlgo::make_shared("suggest_algo_test_group");
  EXPECT_NE(nullptr, algo_group_);
}

TEST_F(SuggestAlgoGroupTest, EmptyParams) {
  algo_group_ = SuggestAlgo::make_shared("suggest_algo_test_group", "");
  EXPECT_EQ(nullptr, algo_group_);
}

TEST_F(SuggestAlgoGroupTest, Sanity) {
  UseThreadPool();
  SetUpDefaults();

  SuggestResponse mock_response_1 = MakeMockSuggestResponse("mock1", 100, 3);
  SuggestResponse mock_response_2 = MakeMockSuggestResponse("mock2", 200, 2);

  EXPECT_CALL(*mock_algo1_, GetCompletions(_, _, _))
      .WillOnce(SetMockResponseNotifyCounter(mock_response_1));

  EXPECT_CALL(*mock_algo2_, GetCompletions(_, _, _))
      .WillOnce(SetMockResponseNotifyCounter(mock_response_2));

  EXPECT_EQ(5, algo_group_->GetCompletions(request_, response_, context_));
}

}  // namespace test
}  // namespace algo
}  // namespace suggest
