// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_SST_SST_READER_H_
#define _PUBLIC_UTIL_STORE_SST_SST_READER_H_

#include <iostream>
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/table.h"
#include "../store.h"

///////////////////////////////////////////////////////////////////////////////
//
// Read-only store to read SSTables. SStable is a sorted string to string map
// with compression / indexing and other goodies used in leveldb.
//
// This class is a read only store that supports:
// - basic functionality
// - forward iteration / sorted forward iteration
// - lower_bound / upper_bound
//
// This store is thread safe. There are no mutator methods (except ctor / dtor).
//
///////////////////////////////////////////////////////////////////////////////

namespace store {

class SstReader : public Store<std::string, std::string> {
 protected:
  class SstReaderIterator : public Store<>::CachingIterator {
   public:
    SstReaderIterator() = default;
    SstReaderIterator(leveldb::Iterator* iter) : iter_(iter) { Update(); }
    virtual bool operator==(const IteratorBase& it) const {
      auto& rhs = dynamic_cast<const SstReaderIterator&>(it);
      return is_end() && rhs.is_end();
    }
    virtual result<bool> operator++() { iter_->Next(); return Update(); }
    virtual result<bool> operator--() { iter_->Prev(); return Update(); }

   private:
    result<bool> Update() {
      if (iter_->Valid()) {
        cache_.reset(new value_type(iter_->key().ToString(), iter_->value().ToString()));
        return true;
      }
      return false;
    }
    bool is_end() const { return !iter_.get() || !iter_->Valid(); }
    std::shared_ptr<leveldb::Iterator> iter_;
  };

 public:
  struct Options : public leveldb::Options {
    Options(const std::string& file) : file(file) {}
    std::string file;
  };

  SstReader(const std::string& filename) : SstReader(Options(filename)) {}

  SstReader(const Options& options) : options_(options) {
    // Get the file.
    leveldb::RandomAccessFile* file;
    leveldb::Status s = leveldb::Env::Default()->NewRandomAccessFile(options_.file, &file);
    if (!s.ok()) { print_error(s.ToString()); return; }

    file_.reset(file);
    // Get file size.
    uint64_t size;
    s = leveldb::Env::Default()->GetFileSize(options_.file, &size);
    if (!s.ok()) { print_error(s.ToString()); return; }

    // Open the file.
    leveldb::Table* table;
    s = leveldb::Table::Open(options_, file_.get(), size, &table);
    if (!s.ok()) { print_error(s.ToString()); return; }

    db_.reset(table);
    init_failed_ = false;
  }

  template<class... Params>
  static SstReader* make_ptr(Params... p) {
    SstReader* ret = new SstReader(p...);
    if (ret->init_failed_) {
      delete ret;
      ret = nullptr;
    }
    return ret;
  }

  ~SstReader() {}

  iterator find(const key_type& k) const {
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->Seek(k);
    return iter->status().ok() && iter->key() == k ?
        Store<>::iterator(new SstReaderIterator(iter)) : end();
  }

  iterator lower_bound(const key_type& k) const {
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->Seek(k);
    return iter->status().ok() ?
        Store<>::iterator(new SstReaderIterator(iter)) : end();
  }

  iterator upper_bound(const key_type& k) const {
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->Seek(k);
    while (iter->Valid() && iter->key() == k) iter->Next();
    return iter->status().ok() ?
        Store<>::iterator(new SstReaderIterator(iter)) : end();
  }

  iterator begin() const {
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->SeekToFirst();
    return Store<>::iterator(new SstReaderIterator(iter));
  }

  iterator end() const {
    return Store<>::iterator(new SstReaderIterator);
  }

 protected:
  void print_error(const std::string& s) {
    std::cerr << "Error: " << s << " file: " << options_.file << std::endl;
  }

  std::shared_ptr<leveldb::RandomAccessFile> file_;
  std::shared_ptr<leveldb::Table> db_;
  Options options_;
  bool init_failed_ = true;
};

}

#endif  // _PUBLIC_UTIL_STORE_SST_SST_READER_H_
