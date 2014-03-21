// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/suggest/server/algos/util/suggest_algo_utils.h"

#include "test/cc/test_main.h"
#include "util/string/stopwords/stopwords_mock.h"

extern string gFlag_stopwords_id;

namespace suggest {
namespace algo {
namespace test {

using ::i18n::StopWords;
using ::i18n::test::MockStopWords;
using ::i18n::test::RegisterNewMockStopWords;

using ::testing::_;
using ::testing::Return;

// Test class for MyTest.
class GetWordMisMatchExtentTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    RegisterNewMockStopWords("mock_stopwords");
    gFlag_stopwords_id = "mock_stopwords";
  }

  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    SetUpDefaults();
  }

  // Sets up default values for all values.
  void SetUpDefaults() {
    mock_stopwords_proxy_ = StopWords::make_shared("mock_stopwords", "en");
    mock_stopwords_ = dynamic_cast<MockStopWords*>(mock_stopwords_proxy_.get());
    ASSERT_NOTNULL(mock_stopwords_);
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  StopWords::mutable_shared_proxy mock_stopwords_proxy_;
  MockStopWords* mock_stopwords_ = nullptr;
};

TEST_F(GetWordMisMatchExtentTest, MismatchScore) {
  // Treat everything as stopword so that we can simply compute mismatch scores.
  EXPECT_CALL(*mock_stopwords_, IsStopWord(_)).WillRepeatedly(Return(true));

  EXPECT_EQ(30.5, GetWordMisMatchExtent("chicago", vector<string>{"ho ", "chi"}));
  EXPECT_EQ(0, GetWordMisMatchExtent("chicago", vector<string>{"chicago"}));
  EXPECT_EQ(0, GetWordMisMatchExtent("chicago", vector<string>{"chi"}));
  EXPECT_EQ(14, GetWordMisMatchExtent("chicago", vector<string>{"chi", "ho"}));
  EXPECT_EQ(42, GetWordMisMatchExtent("chicago", vector<string>{"chi ", "ho"}));
  EXPECT_EQ(49, GetWordMisMatchExtent("chicago", vector<string>{"no", "match"}));

  EXPECT_EQ(23, GetWordMisMatchExtent("chicago hope", vector<string>{"ho", "chi"}));
  EXPECT_EQ(45.5, GetWordMisMatchExtent("chicago hope", vector<string>{"ho ", "chi"}));
  EXPECT_EQ(0, GetWordMisMatchExtent("chicago hope", vector<string>{"chicago hope"}));
  EXPECT_EQ(0, GetWordMisMatchExtent("chicago hope", vector<string>{"chicago"}));
  EXPECT_EQ(0, GetWordMisMatchExtent("chicago hope", vector<string>{"chi"}));
  EXPECT_EQ(56.5, GetWordMisMatchExtent("chicago hope", vector<string>{"chi ", "ho"}));
  EXPECT_EQ(10, GetWordMisMatchExtent("chicago hope", vector<string>{"chi", "ho"}));
  EXPECT_EQ(84, GetWordMisMatchExtent("chicago hope", vector<string>{"no", "match"}));
}

}  // namespace test
}  // namespace algo
}  // namespace suggest
