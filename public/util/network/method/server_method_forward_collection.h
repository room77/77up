// Copyright 2015 Room77 Inc. All Rights Reserved.
// Author: sball@room77.com (Stephen Ball)

#ifndef _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_FORWARD_COLLECTION_H_
#define _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_FORWARD_COLLECTION_H_

#include <functional>
#include <mutex>
#include <unordered_map>

#include "base/common.h"
#include "util/factory/factory.h"
#include "util/hash/hash_util.h"

namespace network {

// Collection of all the forwarded server methods for a particular server.
// We assume all server methods will be registered before the server is started
// and no new ones would be added after that. This allows us to do all gets
// on method handlers without locking the mutex. In case this changes in the
// future, we will have to modify the code accordingly.
class ServerMethodForwardCollection :
    public Factory<ServerMethodForwardCollection> {
 public:
  struct Data {
    string server_type;
    string remote_method;
    int timeout;
  };

  typedef unordered_map<string, Data,
      ::hash::string_casefold_hash, ::hash::string_casefold_eq> ForwardMap;

  ServerMethodForwardCollection() {}

  ServerMethodForwardCollection(const ServerMethodForwardCollection& collection) {
    AddCollection(collection);
  }

  // Returns the proxy for the method collection for the given server.
  static shared_proxy GetCollectionForServer(const string& server_name) {
    shared_proxy proxy = make_shared(server_name);
    if (proxy == nullptr) {
      bind(server_name, [] { return new ServerMethodForwardCollection(); });
      proxy = make_shared(server_name);
    }
    ASSERT_NOTNULL(proxy) << "Could not get collection for server " << server_name;
    return proxy;
  }

  // template with 3 parameters:
  // - input type
  // - output type
  // - RPC function (with operator() that takes "const tInput&" and "tOutput *"
  //                 and returns 0 (for success) or non-zero error code)
  void Register(const string& name, const Data& data) const {
    // sanity checks
    ASSERT(name.size() > 0) << "opname cannot be empty.";
    ASSERT(name.find(' ') == string::npos) << "opname cannot contain space characters.";
    ASSERT(name != "GET")  << "opname cannot be 'GET'.";
    ASSERT(name != "POST") << "opname cannot be 'POST'.";
    ASSERT(name != "URL")  << "opname cannot be 'URL'.";
    ASSERT(name != "URL")  << "opname cannot be 'URL'.";
    ASSERT(name[0] != '_') << "opname cannot start with '_'.";

    lock_guard<std::mutex> l(mutex_);
    // Check for duplicates.
    ASSERT(methods_.find(name) == methods_.end()) << "Duplicate op-name: " << name;

    LOG(INFO) << "Registering remote method: " << name;
    methods_[name] = data;
  }

  // Returns the data for the input method name.
  const Data* GetData(const string& name) const {
    const auto iter = methods_.find(name);
    if (iter != methods_.end()) return &(iter->second);
    return nullptr;
  }

  // Returns the data and registered name for the input method name.
  pair<string, const Data*> GetDataAndName(
      const string& name) const {
    const auto iter = methods_.find(name);
    if (iter != methods_.end()) return {iter->first, &(iter->second)};

    return {name, nullptr};
  }

  const ForwardMap& methods() const { return methods_; }

  int size() const { return methods_.size(); }
  bool empty() const { return methods_.empty(); }

  template<typename C>
  C MethodNames(bool sortnames = true) const {
    C names;
    for (const auto& p : methods_) {
      names.insert(names.end(), p.first);
    }

    if (sortnames)
      sort(names.begin(), names.end(), less<string>());

    return names;
  }

  // Adds the given collection to the current collection.
  void AddCollection(const ServerMethodForwardCollection& collection) {
    for (const auto& p : collection.methods_) methods_.insert(p);
  }

 private:
  mutable ForwardMap methods_;
  mutable std::mutex mutex_;
};

}  // namespace network

#endif  // _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_FORWARD_COLLECTION_H_
