// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/log/offline/reader/raw_user_log_reader.h"

#include "meta/log/offline/reader/log_element_reader.h"
#include "test/cc/test_main.h"
#include "util/time/localtime.h"

namespace logging {
namespace reader {
namespace test {

namespace {

struct CounterLogElementReader : public LogElementMutableReaderInterface {
  // Simply return 0 every time it is called to stop further processing of the file.
  virtual int ReadMutable(const string& element) {
    ++count;
    return 0;
  }

  int count = 0;
};

}  // namespace

// Test class for RawUserLogReader.
class RawUserLogReaderTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {}

  // Tears down the test fixture.
  virtual void TearDown() {}

  CounterLogElementReader element_reader_;
};

TEST_F(RawUserLogReaderTest, Initialization) {
  LocalDate today = LocalDate::Today();
  LocalDate yesterday = today - 1;
  {
    RawUserLogReader reader(1);
    EXPECT_EQ("/home/share/data/logs/raw", reader.dir_name());
    EXPECT_EQ(yesterday, reader.begin_date());
    EXPECT_EQ(today, reader.end_date());
  }
  {
    RawUserLogReader reader(yesterday, today);
    EXPECT_EQ("/home/share/data/logs/raw", reader.dir_name());
    EXPECT_EQ(yesterday, reader.begin_date());
    EXPECT_EQ(today, reader.end_date());
  }
  {
    RawUserLogReader reader(yesterday.PrintFormatted("%Y%m%d"), today.PrintFormatted("%Y%m%d"));
    EXPECT_EQ("/home/share/data/logs/raw", reader.dir_name());
    EXPECT_EQ(yesterday, reader.begin_date());
    EXPECT_EQ(today, reader.end_date());
  }
}

TEST_F(RawUserLogReaderTest, Sanity) {
  RawUserLogReader reader(1);

  // We returned false for each file we parsed. Thus, the returned value should be 0.
  EXPECT_EQ(0, reader.ReadLogs(element_reader_));
  EXPECT_EQ(24, element_reader_.count);
}

}  // namespace test
}  // namespace reader
}  // namespace logging
