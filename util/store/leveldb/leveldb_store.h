// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_LEVELDB_LEVELDB_STORE_H_
#define _PUBLIC_UTIL_STORE_LEVELDB_LEVELDB_STORE_H_

#include <leveldb/db.h>
#include "../store.h"

///////////////////////////////////////////////////////////////////////////////
//
// This is a mutable store wrapper around leveldb.
// Following operations are implemented:
// - find()
// - checking an iterator against end()
// - insert()
// - erase()
// - size()
//
// It implements all required functionalities of basic and mutable stores.
//
///////////////////////////////////////////////////////////////////////////////

namespace store {

class LeveldbStore : public Store<std::string, std::string> {
 protected:
  class LeveldbStoreIterator : public Store<>::CachingIterator {
   public:
    LeveldbStoreIterator() = default;
    LeveldbStoreIterator(leveldb::Iterator* iter) : iter_(iter) { Update(); }
    virtual bool operator==(const IteratorBase& it) const {
      auto& rhs = dynamic_cast<const LeveldbStoreIterator&>(it);
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
    friend class LeveldbStore;
  };

 public:
  // Inherit from leveldb options and add table.
  struct Options : public leveldb::Options {
    Options(const std::string& table) : table(table) { create_if_missing = true; }
    std::string table;     // Tables are simply dedicated directories.
  };

  LeveldbStore(const std::string& dir) : LeveldbStore(Options(dir)) {}

  LeveldbStore(const Options& options) : options_(options) {
    leveldb::Status status = leveldb::DB::Open(options, options_.table, &db_);
    assert(status.ok());
  }

  ~LeveldbStore() { delete db_; }

  iterator find(const key_type& k) const {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->Seek(k);
    return iter->status().ok() && iter->Valid() && iter->key() == k ?
        Store<>::iterator(new LeveldbStoreIterator(iter)) : end();
  }

  iterator lower_bound(const key_type& k) const {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->Seek(k);
    return iter->status().ok() ?
        Store<>::iterator(new LeveldbStoreIterator(iter)) : end();
  }

  iterator upper_bound(const key_type& k) const {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->Seek(k);
    while (iter->Valid() && iter->key() == k) iter->Next();
    return iter->status().ok() ?
        Store<>::iterator(new LeveldbStoreIterator(iter)) : end();
  }

  iterator begin() const {
    leveldb::Iterator* iter = db_->NewIterator(leveldb::ReadOptions());
    iter->SeekToFirst();
    return Store<>::iterator(new LeveldbStoreIterator(iter));
  }

  iterator end() const {
    return Store<>::iterator(new LeveldbStoreIterator);
  }

  result<bool> insert(const Store<>::value_type& v) {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), v.first, v.second);
    return s.ok();
  }

  result<bool> erase(const key_type& k) {
    std::lock_guard<std::recursive_mutex> l(mutex_);
    leveldb::Status s = db_->Delete(leveldb::WriteOptions(), k);
    return s.ok();
  }

 protected:
  leveldb::DB* db_;
  Options options_;
  mutable std::recursive_mutex mutex_;
};

}

#endif  // _PUBLIC_UTIL_STORE_LEVELDB_LEVELDB_STORE_H_
