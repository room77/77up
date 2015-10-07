// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

#include <iostream>
#include <sstream>

#include "util/file/field_reader.h"

#include "test/cc/test_main.h"

template<typename FieldReader=DelimitedFieldReader>
class DelimitedFieldReaderTest : public testing::Test {
 protected:
  virtual void SetUp() {
    tags_ = "TAG1|TAG2|TAG3";
    tags_list_.clear();
    tags_list_.push_back("TAG1");
    tags_list_.push_back("TAG2");
    tags_list_.push_back("TAG3");
    reader_.reset(new FieldReader());
    stream_.reset(new istringstream());
  }

  void SetData(const string& fields) {
    stream_->str(tags_ + "\n" + fields);
  }

  unique_ptr<FieldReader> reader_;
  unique_ptr<istringstream> stream_;
  string tags_;
  vector<string> tags_list_;
};

typedef testing::Types<DelimitedFieldReader, EscapedDelimitedFieldReader>
    AllReaders;
TYPED_TEST_CASE(DelimitedFieldReaderTest, AllReaders);

TYPED_TEST(DelimitedFieldReaderTest, Sanity) {
  this->SetData("v1|v2|12");
  this->reader_->ParseTags(*this->stream_);
  ASSERT(this->reader_->HasTags(this->tags_list_));
  ASSERT(this->reader_->ParseLine(*this->stream_));
  string val;
  ASSERT(this->reader_->Get("TAG1", &val));
  ASSERT_EQ("v1", val);
  ASSERT(this->reader_->Get("TAG2", &val));
  ASSERT_EQ("v2", val);
  int v3;
  ASSERT(this->reader_->Get("TAG3", &v3));
  ASSERT_EQ(12, v3);
}

typedef DelimitedFieldReaderTest<EscapedDelimitedFieldReader>
    EscapedDelimitedFieldReaderTest;

TEST_F(EscapedDelimitedFieldReaderTest, Sanity) {
  this->SetData("v1|\"v2 with \"\"double\"\" quotes\"|\"12\"");
  this->reader_->ParseTags(*this->stream_);
  ASSERT(this->reader_->HasTags(this->tags_list_));
  ASSERT(this->reader_->ParseLine(*this->stream_));
  string val;
  ASSERT(this->reader_->Get("TAG1", &val));
  ASSERT_EQ("v1", val);
  ASSERT(this->reader_->Get("TAG2", &val));
  ASSERT_EQ("v2 with \"double\" quotes", val);
  int v3;
  ASSERT(this->reader_->Get("TAG3", &v3));
  ASSERT_EQ(12, v3);
}

TEST_F(EscapedDelimitedFieldReaderTest, InvalidEspcapes) {
  this->SetData("v\\\"1|\"v2 with \\\"double\\\" quotes\"|\"12\"");
  this->reader_->ParseTags(*this->stream_);
  ASSERT(this->reader_->HasTags(this->tags_list_));
  ASSERT(this->reader_->ParseLine(*this->stream_));
  string val;
  ASSERT(this->reader_->Get("TAG1", &val));
  ASSERT_EQ("v\"1" , val);
  ASSERT(this->reader_->Get("TAG2", &val));
  ASSERT_EQ("v2 with \"double\" quotes", val);
  int v3;
  ASSERT(this->reader_->Get("TAG3", &v3));
  ASSERT_EQ(12, v3);
}
