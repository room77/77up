// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_SST_SST_WRITER_H_
#define _PUBLIC_UTIL_STORE_SST_SST_WRITER_H_

#include <map>
#include <string>
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/env.h>
#include <leveldb/table_builder.h>
#include "../store.h"

///////////////////////////////////////////////////////////////////////////////
//
// Write-only store to build SSTables (Sorted Sting Table) from leveldb.
//
// Keys / values are inserted into a buffer for sorting purposes. Nothing is
// written to disk before the destructor of this class. Instance needs to go out
// of scope to write its data.
//
// Only method this store supports is insert().
//
// This store is thread safe. Accesses to insert() are internally synchronized.
//
///////////////////////////////////////////////////////////////////////////////

namespace store {

class SstWriter : public Store<std::string, std::string> {
 protected:
  class SstWriterIterator : public Store<>::CachingIterator {
   public:
    SstWriterIterator() = default;
    SstWriterIterator(leveldb::Iterator* iter) : iter_(iter) { Update(); }
    virtual bool operator==(const IteratorBase& it) const {
      auto& rhs = dynamic_cast<const SstWriterIterator&>(it);
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

  SstWriter(const std::string& filename) : SstWriter(Options(filename)) {}

  SstWriter(const Options& options) : options_(options) {
    leveldb::WritableFile* file;
    leveldb::Status s = leveldb::Env::Default()->NewWritableFile(options_.file, &file);
    if (!s.ok()) { print_error(s.ToString()); return; }
    file_.reset(file);
    builder_.reset(new leveldb::TableBuilder(options_, file_.get()));
    init_failed_ = false;
  }

  template<class... Params>
  static SstWriter* make_ptr(Params... p) {
    SstWriter* ret = new SstWriter(p...);
    if (ret->init_failed_) {
      delete ret;
      ret = nullptr;
    }
    return ret;
  }

  ~SstWriter() {
    for (auto& p : buffer_) builder_->Add(p.first, p.second);
    assert(builder_->Finish().ok());
  }

  result<bool> insert(const Store<>::value_type& v) {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    return buffer_.insert(v).second;
  }

 protected:
  void print_error(const std::string& s) {
    std::cerr << "Error: " << s << " file: " << options_.file << std::endl;
  }

  std::map<std::string, std::string> buffer_;
  std::shared_ptr<leveldb::WritableFile> file_;
  std::shared_ptr<leveldb::TableBuilder> builder_;
  Options options_;
  bool init_failed_ = true;
  mutable std::recursive_mutex mutex_;
};

}

#endif  // _PUBLIC_UTIL_STORE_SST_SST_WRITER_H_
