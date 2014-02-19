// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

#include <memory>
#include <vector>

#include "util/index/index.h"
#include "util/string/strutil.h"

#include "test/cc/test_main.h"

namespace test {

struct tIndexTestStr {
  string value;

  tIndexTestStr() {}
  explicit tIndexTestStr(const string& str): value(str) {}
};

class IndexTest : public testing::Test {
 protected:
  virtual void SetUp() {
    keys_.push_back("");
    keys_.push_back("Ab Cd e");
    keys_.push_back("ab Cde");
    keys_.push_back("aB Pq");
  }

  void AddDefaultValues() {
    for (const string& key : keys_)
      AddToStorage(key);
  }

  void AddToStorage(const string& str) {
    storage_.push_back(shared_ptr<tIndexTestStr>(new tIndexTestStr(str)));
  }

  vector<shared_ptr<tIndexTestStr> > storage_;
  Index<const char*, shared_ptr<tIndexTestStr> > index_;
  vector<string> keys_;
};

TEST_F(IndexTest, Sanity) {
  AddDefaultValues();
  BUILD_UNIQUE_INDEX_ON_FIELD(storage_, value, index_);
  ASSERT_EQ(keys_.size(), index_.size());

  for (const string& key : keys_) {

    vector<string> lookup_keys;
    lookup_keys.push_back(key);
    lookup_keys.push_back(strutil::ToUpper(key));
    lookup_keys.push_back(strutil::ToLower(key));

    for (const string& lookup_key : lookup_keys) {
      const shared_ptr<tIndexTestStr>& val =
          index_.RetrieveUnique(lookup_key);
      ASSERT_NOTNULL(val);
      ASSERT_EQ(key, val->value);
    }
  }
}

TEST_F(IndexTest, DuplicateKey) {
  AddDefaultValues();
  BUILD_UNIQUE_INDEX_ON_FIELD(storage_, value, index_);
  ASSERT_EQ(keys_.size(), index_.size());

  const shared_ptr<tIndexTestStr>& val = index_.RetrieveUnique(keys_[1]);
  ASSERT_NOTNULL(val);
  ASSERT_EQ(keys_[1], val->value);

  shared_ptr<tIndexTestStr> new_val(new tIndexTestStr(keys_[1] + " New val"));
  ASSERT_DEATH(index_.AddToIndex(strutil::ToLower(keys_[1]), new_val, true),
               "");
}

TEST_F(IndexTest, PrefixKeys) {
  AddDefaultValues();
  BUILD_PREFIX_INDEX_ON_FIELD(storage_, value, index_);
  ASSERT_LT(keys_.size(), index_.size());

  vector<shared_ptr<tIndexTestStr> > vals;
  int num_vals = index_.Retrieve("a", &vals);
  ASSERT_EQ(3, num_vals);

  num_vals = index_.Retrieve("abcde", &vals);
  ASSERT_EQ(2, num_vals);
}

}  // namespace test
