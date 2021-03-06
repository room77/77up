// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/stats/simple_stats.h"

#include <chrono>
#include <limits>
#include <memory>

#include "test/cc/test_main.h"

namespace stats {
namespace test {

namespace {

typedef std::chrono::duration<int, std::ratio<86400>> integer_day;

// Using static and template ensures that all timestamps are returned the same across multiple
// calls for the same number.
template<int N = 0>
uint64_t GetTimeStampForNDaysBefore() {
  static uint64_t timestamp = chrono::duration_cast<chrono::microseconds>(
      chrono::duration_cast<integer_day>(chrono::high_resolution_clock::now().time_since_epoch()) -
                                         integer_day(N)).count();
  return timestamp;
}

// Utility test class for SimpleStats.
class TestSimpleStats : public SimpleStats {
 public:
  using SimpleStats::SimpleStats;

  void SetupMockData() {
    SetMetaData(MockMetaData());
    SetKeyFeq(MockKeyFreqMap());
  }

  void SetKeyFeq(const KeyFreqMap& key_freq) { key_freq_ = key_freq; }
  void SetMetaData(const BasicMetaData& meta_data) { meta_data_ = meta_data; }

  static BasicMetaData MockMetaData() {
    BasicMetaData meta_data;
    meta_data.n = 3;
    meta_data.sum = 6;
    meta_data.min = 1;
    meta_data.max = 3;
    meta_data.squared_sum = 14;
    meta_data.timestamp = GetTimeStampForNDaysBefore<90>();
    return meta_data;
  }

  static SimpleStats::KeyFreqMap MockKeyFreqMap() {
    SimpleStats::KeyFreqMap key_freq_map = {{"key1", 1}, {"key2", 2}, {"key3", 3}};
    return key_freq_map;
  }

  static const string& JSONString() {
    static const string kJSONString = "{\"timestamp\":" +
        to_string(GetTimeStampForNDaysBefore<90>()) + ",\"n\":3,\"sum\":6,\"min\":1,\"max\":3,"
        "\"squared_sum\":14}\n"
        "[[\"key1\",1],[\"key2\",2],[\"key3\",3]]";
    return kJSONString;
  }
};

}  // namespace

// Test class for SimpleStats.
class SimpleStatsTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    stats_.reset(new TestSimpleStats(90));
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  unique_ptr<TestSimpleStats> stats_;
};

TEST_F(SimpleStatsTest, Sanity) {
  stats_->SetupMockData();

  EXPECT_EQ(3, stats_->size());
  EXPECT_EQ(1, stats_->GetKey("key1"));
  EXPECT_EQ(2, stats_->GetKey("key2"));
  EXPECT_EQ(3, stats_->GetKey("key3"));
  EXPECT_EQ(0, stats_->GetKey("key4"));

  const BasicMetaData& md = stats_->meta_data();
  EXPECT_EQ(3, md.n);
  EXPECT_EQ(6, md.sum);
  EXPECT_EQ(1, md.min);
  EXPECT_EQ(3, md.max);
  EXPECT_EQ(14, md.squared_sum);
  EXPECT_EQ(GetTimeStampForNDaysBefore<90>(), md.timestamp);

  // Add to existing key.
  stats_->IncrementKey("key1", 2);
  EXPECT_EQ(3, stats_->size());
  EXPECT_EQ(3, stats_->GetKey("key1"));

  EXPECT_EQ(4, md.n);
  EXPECT_EQ(8, md.sum);
  EXPECT_EQ(1, md.min);
  EXPECT_EQ(3, md.max);
  EXPECT_EQ(18, md.squared_sum);

  // Add a new key.
  stats_->IncrementKey("key4", 10);
  EXPECT_EQ(4, stats_->size());
  EXPECT_EQ(10, stats_->GetKey("key4"));

  EXPECT_EQ(5, md.n);
  EXPECT_EQ(18, md.sum);
  EXPECT_EQ(1, md.min);
  EXPECT_EQ(10, md.max);
  EXPECT_EQ(118, md.squared_sum);
}

TEST_F(SimpleStatsTest, TimeDecayDataSanity) {
  stats_->SetupMockData();
  EXPECT_TRUE(stats_->TimeDecayData(GetTimeStampForNDaysBefore<0>()));
  EXPECT_EQ(3, stats_->size());
  EXPECT_DOUBLE_EQ(0.5, stats_->GetKey("key1"));
  EXPECT_DOUBLE_EQ(1, stats_->GetKey("key2"));
  EXPECT_DOUBLE_EQ(1.5, stats_->GetKey("key3"));

  const BasicMetaData& md = stats_->meta_data();
  EXPECT_DOUBLE_EQ(3, md.n);
  EXPECT_DOUBLE_EQ(3, md.sum);
  EXPECT_DOUBLE_EQ(0.5, md.min);
  EXPECT_DOUBLE_EQ(1.5, md.max);
  EXPECT_DOUBLE_EQ(3.5, md.squared_sum);
  EXPECT_EQ(GetTimeStampForNDaysBefore<0>(), md.timestamp);
  EXPECT_EQ(GetTimeStampForNDaysBefore<90>(), md.read_ahead_from_time);
}

TEST_F(SimpleStatsTest, TimeDecayDataInvalidNow) {
  stats_->SetupMockData();
  EXPECT_FALSE(stats_->TimeDecayData(GetTimeStampForNDaysBefore<120>()));
  EXPECT_EQ(3, stats_->size());
  EXPECT_EQ(1, stats_->GetKey("key1"));
  EXPECT_EQ(2, stats_->GetKey("key2"));
  EXPECT_EQ(3, stats_->GetKey("key3"));
  EXPECT_EQ(0, stats_->GetKey("key4"));

  const BasicMetaData& md = stats_->meta_data();
  EXPECT_EQ(3, md.n);
  EXPECT_EQ(6, md.sum);
  EXPECT_EQ(1, md.min);
  EXPECT_EQ(3, md.max);
  EXPECT_EQ(14, md.squared_sum);
  EXPECT_EQ(GetTimeStampForNDaysBefore<90>(), md.timestamp);
}

TEST_F(SimpleStatsTest, TimeDecayDataEmpty) {
  stats_->SetupMockData();
  stats_->SetKeyFeq({});
  EXPECT_EQ(0, stats_->size());

  EXPECT_TRUE(stats_->TimeDecayData(GetTimeStampForNDaysBefore<0>()));

  const BasicMetaData& md = stats_->meta_data();
  EXPECT_EQ(0, md.n);
  EXPECT_EQ(0, md.sum);
  EXPECT_EQ(numeric_limits<int>::max(), md.min);
  EXPECT_EQ(0, md.max);
  EXPECT_EQ(0, md.squared_sum);
  EXPECT_EQ(GetTimeStampForNDaysBefore<0>(), md.timestamp);
  EXPECT_EQ(GetTimeStampForNDaysBefore<90>(), md.read_ahead_from_time);
}

TEST_F(SimpleStatsTest, ReadUptoTime) {
  stats_->SetupMockData();
  EXPECT_EQ(0, stats_->meta_data().read_ahead_from_time);
  EXPECT_EQ(GetTimeStampForNDaysBefore<90>(), stats_->ReadUptoTime());

  BasicMetaData md = stats_->MockMetaData();
  md.read_ahead_from_time = GetTimeStampForNDaysBefore<120>();
  stats_->SetMetaData(md);
  EXPECT_EQ(GetTimeStampForNDaysBefore<120>(), stats_->ReadUptoTime());
}

TEST_F(SimpleStatsTest, IncrementKeyWithTimestamp) {
  stats_->SetupMockData();
  EXPECT_TRUE(stats_->TimeDecayData(GetTimeStampForNDaysBefore<0>()));

  // Set a new key, with value from 15 days ago.
  stats_->IncrementKeyWithTimestamp("key4", GetTimeStampForNDaysBefore<15>(), 1);
  EXPECT_DOUBLE_EQ(0.89089871814033927, stats_->GetKey("key4"));

  // Set a new key, with value from 0 days ago.
  stats_->IncrementKeyWithTimestamp("key5", GetTimeStampForNDaysBefore<0>(), 2);
  EXPECT_DOUBLE_EQ(2, stats_->GetKey("key5"));

  EXPECT_EQ(1, stats_->GetKey("key2"));
  // Try setting a value more than 90 days before. This should have no effect.
  stats_->IncrementKeyWithTimestamp("key2", GetTimeStampForNDaysBefore<120>(), 1);
  EXPECT_EQ(1, stats_->GetKey("key2"));

  // Try setting a value less than 90 days before. This should get added.
  stats_->IncrementKeyWithTimestamp("key2", GetTimeStampForNDaysBefore<30>(), 1);
  EXPECT_DOUBLE_EQ(1.7937005259840997, stats_->GetKey("key2"));
}

TEST_F(SimpleStatsTest, RemoveKeysBelowThreshold) {
  stats_->SetupMockData();
  EXPECT_EQ(3, stats_->size());

  EXPECT_EQ(0, stats_->RemoveKeysBelowThreshold(0));
  EXPECT_EQ(3, stats_->size());

  EXPECT_EQ(0, stats_->RemoveKeysBelowThreshold(1));
  EXPECT_EQ(3, stats_->size());

  EXPECT_EQ(1, stats_->RemoveKeysBelowThreshold(1.5));
  EXPECT_EQ(2, stats_->size());

  const BasicMetaData& md = stats_->meta_data();
  EXPECT_EQ(2, md.n);
  EXPECT_EQ(5, md.sum);
  EXPECT_EQ(2, md.min);
  EXPECT_EQ(3, md.max);
  EXPECT_EQ(13, md.squared_sum);
}

TEST_F(SimpleStatsTest, Serialization) {
  stats_->SetupMockData();
  EXPECT_EQ(stats_->JSONString(), stats_->ToJSON());
}

TEST_F(SimpleStatsTest, DeSerialization) {
  EXPECT_TRUE(stats_->FromJSON(stats_->JSONString()));
  EXPECT_EQ(stats_->JSONString(), stats_->ToJSON());
}

}  // namespace test
}  // namespace stats
