// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

// Disk based persistent cache library with locality. We use one file per
// locality group. A functor should be provided to map keys to groups during
// initialization of the class.

// Cache size can be controlled via the policy object. When cache size hits the
// maximum size, least recently used group is flushed to disk (if dirty) and
// dropped from cache.

// Internally, the class caches groups and marks modified groups as dirty. This
// means that it is possible to send a bunch of write requests to keys within
// the same group relatively efficiently (does not involve a disk write until
// the group is flushed).

// Current implementation does not flush items periodically. Cache size check
// is only done upon insert() and find() operations. It may be a good idea to
// explicitly flush() or clear() the cache periodically.

// It is possible to locally shard the groups so that mutex locked operations
// lock only a single shard rather than the whole cache.

#ifndef _PUBLIC_UTIL_CACHE_GROUP_CACHE_H_
#define _PUBLIC_UTIL_CACHE_GROUP_CACHE_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <deque>
#include <thread>
#include <mutex>
#include <memory>
#include <list>
#include <map>
#include <set>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <cstdio>

#include "base/common.h"
#include "util/file/file.h"
#include "util/factory/factory.h"
#include "util/serial/serializer.h"

class GroupCache : public Factory<GroupCache> {
 public:
  // Example "functor" base class that follows the grouper format.
  // We have the following modes of operation:
  // - Call the constructor with a valid function<stirng(const string&)>.
  // - Derive from this class, register derived instance with an id, and call
  //   the constructor with the id.
  class Grouper : public Factory<Grouper> {
   public:
    Grouper() {}
    Grouper(function<string(const string&)> func) : func_(func) {}
    Grouper(const string& grouper_id) : Grouper(grouper_id.c_str()) {}
    Grouper(const char* grouper_id) {
      auto grouper = Grouper::make_shared(grouper_id);
      ASSERT(grouper.get()) << "Could not initialize grouper id: " << grouper_id;
      func_ = [grouper](const string& key) { return (*grouper)(key); };
    }

    // Override this method in derived classes.
    virtual string operator()(const string& key) const { return func_(key); }

   private:
    function<string(const string&)> func_;
  };

  struct value_type {
    value_type(const string& key = "", const string& data = "", uint64_t time = 0)
      : key(key), data(new string(data)), time(time) {}
    string key;
    shared_ptr<string> data;
    fixedint<uint64_t> time;

    bool operator == (const value_type& v) const {
      return key == v.key && data == v.data && time == v.time;
    }

    // High level I/O methods to read/write entries in binary format.
    bool Read(istream& is) {
      return serial::Serializer::FromRawBinary(is, this);
    }

    bool Write(ostream& os) {
      serial::Serializer::ToRawBinary(os, *this);
      return os.good();
    }
    SERIALIZE(key*1 / data*2 / time*3);
  };

 private:
  typedef lock_guard<recursive_mutex> lock_t;
  typedef unordered_map<string, shared_ptr<value_type>> group_t;
  typedef list<pair<string, shared_ptr<group_t>>> list_t;

 public:
  struct Policy {
    Policy(int max_cache_size = 1000, int max_life = -1, int shards = 0)
        : max_cache_size(max_cache_size), max_life(max_life), num_local_shards(shards) {}
    int max_cache_size;     // Do not retain more than this many groups in cache.
    int max_life;           // Only retain items fresher than this many seconds.
    int num_local_shards = 0;
  };

  typedef string key_type;
  typedef string data_type;
  typedef size_t size_type;
  typedef chrono::steady_clock clock_type;

  // Minimal iterator definition. We only support checking against end().
  struct iterator {
    iterator() = default;
    iterator(shared_ptr<value_type>& data) : data_(data) {}
    bool operator == (const iterator& it) const {
      return end() && it.end() || !end() && !it.end() && *data_ == *it.data_;
    }
    bool operator != (const iterator& it) const { return !(*this == it); }
    const value_type& operator*()  const        { return *data_; }
    const value_type* operator->() const        { return &*data_; }

   private:
    bool end() const { return data_.get() == nullptr; }
    shared_ptr<const value_type> data_;
  };

  typedef iterator const_iterator;

  GroupCache(const string& base_path, const Grouper& grouper,
             Policy policy = Policy())
      : base_path_(base_path), default_grouper_(grouper), policy_(policy) {
    if (policy_.num_local_shards) {
      Policy shard_policy = policy_;
      shard_policy.num_local_shards = 0;
      shards_.resize(policy_.num_local_shards);
      for (auto& shard : shards_)
        shard.reset(new GroupCache(base_path, grouper, shard_policy));
    }
  }

  virtual ~GroupCache() { flush(); }

  // Recommended insert API.
  pair<iterator, bool> insert(const key_type& k, const data_type& d) {
    return insert(value_type(k, d, Now()), default_grouper_);
  }

  // Recommended insert API with non-default grouper.
  pair<iterator, bool> insert(const key_type& k, const data_type& d, const Grouper& grouper ) {
    return insert(value_type(k, d, Now()), grouper);
  }

  pair<iterator, bool> insert(const value_type& v, const Grouper& grouper) {
    string group = grouper(v.key);
    return (group.empty()) ? make_pair(end(), false) : insert(v, group);
  }

  // This would also work but the above versions are probably more convenient.
  pair<iterator, bool> insert(const value_type& v, const string& group) {
    if (!shards_.empty()) return GroupToShard(group).insert(v, group);
    lock_t l(mutex_);
    if (cache_.find(group) == cache_.end()) ReadGroup(group);
    SetDirty(group);
    GetOrCreateCacheEntry(group)[v.key].reset(new value_type(v));
    auto ret = make_pair(iterator(GetOrCreateCacheEntry(group)[v.key]), true);
    AdjustSize();
    return ret;
  }

  // Erase API.
  void erase(const key_type& k) { erase(k, default_grouper_); }

  void erase(const key_type& k, const Grouper& grouper) {
    string group = grouper(k);
    if (group.empty()) {
      LOG(INFO) << "Could not determine group for key: " << k;
      return;
    };
    return erase(k, group);
  }

  // Erase with non-default grouper.
  void erase(const key_type& k, const string& group) {
    if (!shards_.empty()) return GroupToShard(group).erase(k, group);
    lock_t l(mutex_);
    if (cache_.find(group) == cache_.end()) ReadGroup(group);
    GetOrCreateCacheEntry(group).erase(k);
    SetDirty(group);
  }

  // Search API. If the group the key belongs to is not cached, reads it from
  // disk and caches it.
  iterator find(const key_type& k) const { return find(k, default_grouper_); }

  iterator find(const key_type& k, const Grouper& grouper) const {
    string group = grouper(k);
    return (group.empty()) ? end() : find(k, group);
  }

  iterator find(const key_type& k, const string& group) const {
    if (!shards_.empty()) return GroupToShard(group).find(k, group);
    lock_t l(mutex_);
    if (cache_.find(group) == cache_.end()) ReadGroup(group);
    auto it = cache_.find(group);
    if (it == cache_.end()) return iterator();
    group_t& m = GetOrCreateCacheEntry(group);
    auto jt = m.find(k);
    if (jt != m.end() && IsExpired(*jt->second)) {
      m.erase(k);
      jt = m.end();
      SetDirty(group);
    }
    auto ret = (jt == m.end()) ? iterator() : iterator(jt->second);
    AdjustSize();
    return ret;
  }

  typedef const function<void(const value_type&)>& find_function;

  // For each key that is found, call find_function with the value_type.
  void find(const vector<key_type>& keys, find_function f) const {
    return find(keys, f, default_grouper_);
  }

  void find(const vector<key_type>& keys, find_function f, const Grouper& grouper) const {
    map<string, vector<string>> group_to_keys;
    for (auto& k : keys) {
      string group = grouper(k);
      if (!group.empty()) group_to_keys[group].push_back(k);
    }
    for (auto& gk : group_to_keys) find(gk.second, f, gk.first);
  }

  void find(const vector<key_type>& keys, find_function f, const string& group) const {
    if (!shards_.empty()) return GroupToShard(group).find(keys, f, group);
    lock_t l(mutex_);
    if (cache_.find(group) == cache_.end()) ReadGroup(group);
    auto it = cache_.find(group);
    list<iterator> ret;
    if (it == cache_.end()) return;
    group_t& m = GetOrCreateCacheEntry(group);
    for (const auto& k : keys) {
      auto jt = m.find(k);
      if (jt != m.end() && IsExpired(*jt->second)) {
        m.erase(k);
        jt = m.end();
        SetDirty(group);
      }
      if (jt != m.end()) f(*jt->second);
    }
    AdjustSize();
  }

  // Policy can be modified on the fly if needed, but it is recommended to
  // flush() / clear() before doing so.
  Policy& policy()      { return policy_; }

  iterator end() const  { return iterator(); }

  // Returns current time in micro seconds.
  static uint64_t Now() {
    return chrono::duration_cast<chrono::microseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count();
  }

  // Flushes the data but does not clear the cache. Returns number of groups
  // written to disk.
  int flush() {
    if (!shards_.empty()) {
      int num_dirty = 0;
      for (auto& shard : shards_) num_dirty += shard->flush();
      return num_dirty;
    }
    lock_t l(mutex_);
    // Make a copy of dirty list since we cannot use dirty_ directly
    // (FlushGroup() will erase stuff changing the container underneat).
    auto dirty = dirty_;
    // LOG(INFO) << "Flushing " << dirty.size() << " groups";
    for (auto& group : dirty) FlushGroup(group);
    ASSERT(dirty_.empty());
    file_not_found_.clear();
    return dirty.size();
  }

  // Flushes a dirty item with a probability prob (must be between 0 to 100).
  int flush(int prob) {
    if (!shards_.empty()) {
      int num_dirty = 0;
      for (auto& shard : shards_) num_dirty += shard->flush(prob);
      return num_dirty;
    }
    // Make a copy of dirty list since we cannot use dirty_ directly
    // (FlushGroup() will erase stuff changing the container underneat).
    deque<string> dirty;
    {
      lock_t l(mutex_);
      for (auto& group : dirty_) if (rand_() % 100 < prob) dirty.push_back(group);
    }
    for (auto& group : dirty) {
      lock_t l(mutex_);
      FlushGroup(group);
    }
    file_not_found_.clear();
    return dirty.size();
  }

  // Flushes the data and clears the cache. Returns number of groups written to
  // disk.
  int clear() {
    if (!shards_.empty()) {
      int num_dirty = 0;
      for (auto& shard : shards_) num_dirty += shard->clear();
      return num_dirty;
    }
    lock_t l(mutex_);
    int num = flush();
    ASSERT(dirty_.empty());
    cache_.clear();
    list_.clear();
    return num;
  }

  // Returns the number of cached groups.
  size_t size() {
    if (!shards_.empty()) {
      int size = 0;
      for (auto& shard : shards_) size += shard->size();
      return size;
    }
    return cache_.size();
  }

  // Dumps basic stats about the cache for debugging purposes.
  void DumpStats(ostream& out = cout) const {
    if (!shards_.empty()) {
      for (int i = 0; i < shards_.size(); ++i) {
        out << "Shard " << i << " {" << endl;
        shards_[i]->DumpStats(out);
        out << "}" << endl;
      }
      return;
    }
    out << "  Dirty  groups: ";
    for (auto& group : dirty_) out << group << " ";
    out << endl;
    out << "  Cached groups: ";
    for (auto& p : cache_) out << p.first << " ";
    out << endl;
  }

 protected:
  // Mutex needs to be locked before calling any of the following methods.
  bool IsExpired(const value_type& v) const {
    if (policy_.max_life <= 0) return false;
    bool ret = Now() - v.time > policy_.max_life * 1000000;
    if (ret) VLOG(4) << "Expired key: " << v.key;
    return ret;
  }

  void SetDirty(const string& group) const {
    dirty_.insert(group);
    VLOG(4) << "Set dirty: " << group;
  }

  void SetClean(const string& group) const {
    dirty_.erase(group);
    VLOG(4) << "Set clean: " << group;
  }

  void FlushGroup(const string& group) const {
    VLOG(3) << "Flushing group: " << group;
    if (dirty_.find(group) != dirty_.end()) {
      auto it = cache_.find(group);
      ASSERT(it != cache_.end());
      WriteGroup(group);
      SetClean(group);
    }
    ASSERT(dirty_.find(group) == dirty_.end());
  }

  // Attempt to read a group from disk. Return false on failure.
  bool ReadGroup(const string& group) const {
    ifstream file;
    // Use a custom buffer size.
    char buffer[512 * 1024 + 1];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
    file.open((base_path_ + "/" + group).c_str());
    if (file_not_found_.find(group) != file_not_found_.end()) return false;
    else if (file.fail()) {
      file_not_found_.insert(group);
      return false;
    }
    VLOG(3) << "Reading group: " << group;

    // This is needed for correct behavior in case everything that we attempt to
    // read is deemed expired (still want an empty and dirty entry in cache).
    GetOrCreateCacheEntry(group);
    bool dirty = false;
    for (;;) {
      value_type entry;
      if (!entry.Read(file)) break;
      if (IsExpired(entry)) {
        dirty = true;
        continue;
      }
      GetOrCreateCacheEntry(group)[entry.key].reset(new value_type(entry));
    }
    if (dirty) SetDirty(group);
    return true;
  }

  // Attempt to write a group to disk. Return false on failure.
  bool WriteGroup(const string& group) const {
    auto it = cache_.find(group);
    if (it == cache_.end()) return false;
    bool written = false;
    string path = base_path_ + "/" + group;
    if (!it->second->second->empty()) {
      ofstream file(path.c_str());
      if (file.fail()) {
        // This is used only if the parent directory is not present.
        // If the system call becomes a bottleneck we can replace it.
        // system(("mkdir -p `dirname " + path + "`").c_str());
        file::CreateDirectoryIfNecessary(path);
        file.open(path.c_str());
        ASSERT(file.good()) << "Could not create directory for file: " << path;
      }
      VLOG(3) << "Writing group: " << group;
      // We don't call GetOrCreateCacheEntry since we do not need to change this
      // entry's order in the list. Note: AdjustSize() relies on this behavior.
      stringstream temp_buffer;
      for (auto& p : *it->second->second) {
        if (IsExpired(*p.second)) continue;
        p.second->Write(temp_buffer);
        written = true;
      }
      file << temp_buffer.str();
    }
    if (!written) {
      VLOG(3) << "Deleting group: " << group;
      remove(path.c_str());
    }
    file_not_found_.erase(group);
    return true;
  }

  // Gets a group from cache updating LRU book keeping structures.
  // Creates an entry if one does not exist.
  group_t& GetOrCreateCacheEntry(const string& group) const {
    auto it = cache_.find(group);
    if (it == cache_.end()) {
      list_.push_front(make_pair(group, shared_ptr<group_t>(new group_t)));
      cache_[group] = list_.begin();
      ASSERT_EQ(list_.size(), cache_.size());
    }
    else {
      shared_ptr<group_t> entry = it->second->second;
      list_.erase(it->second);
      list_.push_front(make_pair(group, entry));
      it->second = list_.begin();
      ASSERT_EQ(list_.size(), cache_.size());
    }
    return *list_.begin()->second;
  }

  // Shrink size if we exceeded maximum allowed cache size.
  void AdjustSize() const {
    if (policy_.max_cache_size <= 0 ||
        cache_.size() <= policy_.max_cache_size ||
        list_.empty())
      return;

    // Launch the update in a separate thread.
    thread(
      [this](){
        while (cache_.size() > policy_.max_cache_size) {
          lock_t l(mutex_);
          if (!cache_.size()) continue;
          string group = list_.rbegin()->first;
          FlushGroup(group);
          cache_.erase(group);
          ASSERT(list_.rbegin()->first == group);
          list_.pop_back();
          ASSERT(dirty_.find(group) == dirty_.end());
        }
      }
    ).detach();
  }

  GroupCache& GroupToShard(const string& group) const {
    return *shards_[hash<string>()(group) % policy_.num_local_shards];
  }

  string base_path_;
  Grouper default_grouper_;
  Policy policy_;
  mutable recursive_mutex mutex_;
  mutable unordered_set<string> dirty_;
  mutable unordered_set<string> file_not_found_;
  mutable list_t list_;
  mutable unordered_map<string, list_t::iterator> cache_;
  vector<unique_ptr<GroupCache>> shards_;
  default_random_engine rand_ = default_random_engine(random_device()());
};

#endif  // _PUBLIC_UTIL_CACHE_GROUP_CACHE_H_
