// Copyright 2011 B. Uygar Oztekin
// Copied and modified from Uygar's personal libraries.

// Released to public as part of Room77 opensource initiative.

#ifndef _PUBLIC_UTIL_FACTORY_FACTORY_H_
#define _PUBLIC_UTIL_FACTORY_FACTORY_H_

#include <algorithm>
#include <cassert>
#include <thread>
#include <mutex>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include "mutable_ptr.h"
#include "tuple_util.h"

// This class implements a thread safe, flexible, convenient and generic factory
// implementation. It can be used with practically any class hierarchy by using
// custom allocator functions. Provided that std::shared_ptr reference counting
// and book keeping is thread safe, and the registered classes are thread safe,
// this approach can be used efficiently and safely.

// Assume that we have the following class hierarchy:
// Base -> Derived1()
//      -> Derived2(int)
//      -> Derived3(const string&)

// Derived1 has a default constructor, Derived2's ctor takes an int parameter,
// and Derived3's ctor takes a string that contains a configuration string in
// JSON format for an internal configuration struct.

// We can register creator functors for the above derived classes as follows:

// Let's first create a couple of commandline flags to configure the behavior
// of two of the derived classes we will be registering:

/*

FLAG_int(d2_param, 100, "Int param for d2.");
FLAG_string(d3_config, "{\"name\":\"Hello World!\", \"var\":10 }", "d3 params");

*/

// Using the C++0x lambda syntax we can auto register a named creator for the
// above derived classes as follows:
// in the global scope (outside of main):

/*

auto reg_d1 = Factory<Base>::bind("d1", []{ return new Derived1});
auto reg_d2 = Factory<Base>::bind("d2", []{ return new Derived2(gFlag_d2_param); });
auto reg_d3 = Factory<Base>::bind("d3", []{ return new Derived3(gFlag_d3_config); });

*/

// Let's assume the above was done in a .cc file in a library. Whenever this
// library is linked, the above 3 instances will automatically be registered and
// available for use.

// Whithin a function etc.:

/*

{
  auto p1 = Factory<Base>::make_shared("d1");
  ASSERT(p1.get()) << "Error creating the proxy.";
  // From now on you can use p1 as a smart pointer to the shared Derived1 class
  // registered as "d1".
  p1->SomeVirtualFunction();

  // Moreover, assuming that Derived1 is thread safe, we can create another
  // shared proxy (possibly in another function or thread):
  auto p2 = Factory<Base>::make_shared("d1");
  // p1 and p2 points to the same instance. If creating a Derived1 instance is
  // relatively expensive (e.g. reading a data file etc.), creating p2 once we
  // have p1 should be pretty fast (mutex lock, map lookup, refc update etc.).
  p2->SomeVirtualFunction();
  // p1 and p2 go out of scope here, instance will be deallocated.
}

*/

// As long as there is at least one "proxy" (smart pointer) to a named instance,
// the instance will be kept and subsequent calls to make_share for the same ID
// will return a proxy to the shared instance. Once all smart pointers for a
// given ID go out of scope, the instance will be deallocated. If you like to
// have a singleton like experience, feel free to create a "pinner" proxy to the
// instance to be pinned and keep it alive as long as you need to. Any other
// place asking for the same ID will use the cached / shared instance.

// If your classes are not thread safe, you may want to use the make_unique()
// interface instead. make_unique() behaves similarly to make_shared() but
// always returns a new, unshared instance.

// Note that make_shared always returns a smart pointer to a const instance, so
// you can only call const class methods. This also helps with thread safety.
// If you have to call a non-const method, or implementation of a method
// requires mutable member variables, there is a potential thread safety issue
// that may need your attention. make_unique() points to a mutable instance
// since it is not shared.

// There are two main approaches to use the registry class.
// Use a separate Factory class for the Base class (e.g. Factory<Base>), or if
// you have control over the Base class, it may actually inherit from the
// Factory<Base> class:
//
// class Base : public Factory<Base> { ... };
//
// The latter allows the use of factory functions such as make_shared(), through
// the Base class: auto p1 = Base::make_shared("d1");

// Public API of the Factory class follows C++ standard library naming
// conventions since methods such as make_shared, bind etc. resembles other APIs
// the standard has, and the proxy classes we currently use are std::shared_ptr
// and std::unique_ptr.

// The above usage is the simplest way to use the factory and assumes that
// instances will NOT be configured on the fly other than the parameters they
// are registered with. There is also a way to have "parameterized factories"
// that allow arbitrary number / types of parameters (both at registration time
// as default parameters, as well as runtime as on-the-fly parameters).
// Here is an example:

/*

struct Base : Factory<Base string, string, int, char> {
  // Classes accept 3 parameters:
  Base(const string& p1, int p2, char p3) { ... }
  ...
};

struct Derived : public Base {
  ...
};

auto reg_derived = Base::bind("derived", "default", 10, 't',
    [](const string& str, int i, char c){ return new Derived(str, i, c); });

*/

// Factory<Base string, string, int, char> defines a factory for base class Base
// with string IDs, and string, int and char as parameters. You can have
// arbitrary number of parameters (including none). If you have parameters, the
// creator function that is passed, is expected to receive those parameters as
// well. See the lamda function above.

// The default parameters passed via the bind call are remembered and associated
// with the ID and we can now create IDs with extra on-the-fly parameters:

/*

// This uses the default parameters ("default", 10, 't').
auto p0 = Factory<Base, string, string, int, char>::make_shared("derived");

// This overrides the first 2 parameters and uses ("test1", 1, 't').
// Note that we use the default value for the last parameter.
auto p1 = Factory<Base, string, string, int, char>::make_shared("derived", "test1", 1);

// This creates another proxy with exactly the same parameters as p1.
// The instances will be shared.
auto p2 = Factory<Base, string, string, int, char>::make_shared("derived", "test1", 1);

// This overrides only the first parameter. We will use ("test3", 10, 't').
auto p3 = Factory<Base, string, string, int, char>::make_shared("derived", "test3");

*/

// Even though all proxies use the same ID (registered derived class), they may
// be called with different on-the-fly parameters. In such cases, in addition to
// IDs, reference counting also take into account the parameters used while
// creating shared instances. Note that parameters may change the behavior of
// the class (e.g. read one file, vs. another file).

// In the above example, separate instance are created for p0, p1, and p3.
// Since p2 was called with the same parameters as p1, p1 and p2 share the same
// instance.

// factory_test.cc is a good place to look for usage examples / ideas.

template<class T, class Key = std::string, class... Params>
class Factory {
  struct allocator_base {
    virtual T* operator()(Params... p) const = 0;
  };

  template<class Functor>
  struct allocator : public allocator_base {
    allocator(Functor f) : f(f) {}
    virtual T* operator()(Params... p) const { return f(p...); }
    Functor f;
  };

  typedef std::tuple<Params...> params_type;

  struct registry_entry {
    std::unique_ptr<const allocator_base> creator;
    params_type default_params;
    struct instance_entry {
      std::weak_ptr<const T> wptr;
      bool initializing;
    };
    std::map<params_type, instance_entry> instances;
  };

  typedef std::mutex mutex_type;
  typedef std::map<Key, std::shared_ptr<registry_entry>> registry_type;

  // TODO: Implement the deleter functionality.
  struct deleter {
    deleter(const Key& k, Params... p) : k(k), p(p...) {}
    Key k;
    params_type p;
  };

 public:
  typedef std::shared_ptr<const T> shared_proxy;
  typedef mutable_ptr<T, std::shared_ptr<const T> > mutable_shared_proxy;
  typedef std::unique_ptr<T> unique_proxy;

  template<class Functor>
  static bool bind(const Key& k, Params... p, Functor f) {
    //LOG(INFO) << *k;
    std::lock_guard<mutex_type> l(mutex());
    std::shared_ptr<registry_entry> re(new registry_entry);
    re->creator = std::unique_ptr<const allocator_base>(new allocator<Functor>(f));
    re->default_params = std::tuple<Params...>(p...);
    return registry().insert(std::make_pair(k, re)).second;
  }

  static bool alias(const Key& alias, const Key& k) {
    std::lock_guard<mutex_type> l(mutex());
    auto it = registry().find(k);
    return registry().insert(std::make_pair(alias, it->second)).second;
  }

  // Create a shared instance using default (registered) parameters.
  static shared_proxy make_shared(const Key& k) {
    mutex().lock();
    auto it = registry().find(k);
    if (it == registry().end()) {
      mutex().unlock();
      return nullptr;
    }
    auto params = it->second->default_params;
    mutex().unlock();
    return make_shared_from_tuple(k, params);
  }

  // Create a shared instance by overriding all or a subset of parameters.
  // Order and type of parameters must match respective entries in params_type
  // but it is possible to stop early in which case rest of the parameters will
  // use default registered values.
  template<class Head, class... Tail>
  static shared_proxy make_shared(const Key& k, Head h, Tail... t) {
    mutex().lock();
    auto it = registry().find(k);
    if (it == registry().end()) {
      mutex().unlock();
      return nullptr;
    }
    auto params = it->second->default_params;
    mutex().unlock();
    tuple_util::tuple_merge(params, std::tuple_cat(std::make_tuple(h), std::make_tuple(t...)));
    return make_shared_from_tuple(k, params);
  }

  // Return a sahred proxy while forcing to recreate the instance. Existing
  // proxies to the same key are not affected. Proxies created after this call
  // finishes will use the new instance. There is no "downtime" on the key from
  // the perspective of other callers. They either get the old instance or the
  // new instance. The new instance is created by this thread while the mutex is
  // unlocked, and then, the shared weak_ptr is replaced.
  static shared_proxy make_updated(const Key& k) {
    std::lock_guard<mutex_type> l(mutex());
    auto it = registry().find(k);
    if (it == registry().end()) return nullptr;
    auto params = it->second->default_params;
    mutex().unlock();
    return make_updated_from_tuple(k, params);
  }

  template<class Head, class... Tail>
  static shared_proxy make_updated(const Key& k, Head h, Tail... t) {
    std::lock_guard<mutex_type> l(mutex());
    auto it = registry().find(k);
    if (it == registry().end()) return nullptr;
    auto params = it->second->default_params;
    mutex().unlock();
    tuple_util::tuple_merge(params, std::tuple_cat(std::make_tuple(h), std::make_tuple(t...)));
    return make_updated_from_tuple(k, params);
  }

  // Create a new unshared instance and return a unique_proxy to it by
  // specifying all parameters.
  static unique_proxy make_unique(const Key& k) {
    mutex().lock();
    auto it = registry().find(k);
    if (it == registry().end()) {
      mutex().unlock();
      return nullptr;
    }
    auto params = it->second->default_params;
    mutex().unlock();
    return make_unique_from_tuple(k, params);
  }

  // Create a new unshared instance and return a unique_proxy to it by
  // specifying all parameters.
  template<class Head, class... Tail>
  static unique_proxy make_unique(const Key& k, Head h, Tail... t) {
    mutex().lock();
    auto it = registry().find(k);
    if (it == registry().end()) {
      mutex().unlock();
      return nullptr;
    }
    auto params = it->second->default_params;
    mutex().unlock();
    tuple_util::tuple_merge(params, std::tuple_cat(std::make_tuple(h), std::make_tuple(t...)));
    return make_unique_from_tuple(k, params);
  }

  // Returns the registered keys.
  template<template<class, class ...> class Container>
  static Container<Key> keys() {
    // Take a snapshot of registered keys with minimal locking.
    mutex().lock();
    registry_type reg = registry();
    mutex().unlock();
    Container<Key> ret;
    for (const auto& p : reg) ret.insert(ret.end(), p.first);
    return ret;
  }

  // DEPRECATED. DO NOT use this version unless absolutely necessary.
  // There is a bug in GCC where keys() above may not always compile when
  // multiple templates and typedefs are involved. We don't know why. We will
  // get rid of this version once the bug is resolved.
  template<class Container>
  static void append_keys(Container& ret) {
    // Take a snapshot of registered keys with minimal locking.
    mutex().lock();
    registry_type reg = registry();
    mutex().unlock();
    for (const auto& p : reg) ret.insert(ret.end(), p.first);
  }

  // Returns an associative container from keys to default parameters associated
  // with the keys.
  template<template<class, class, class ...> class Container>
  static Container<Key, params_type> key_values() {
    // Take a snapshot of registered keys with minimal locking.
    mutex().lock();
    registry_type reg = registry();
    mutex().unlock();
    Container<Key, params_type> ret;
    for (const auto& p : reg) ret.insert(ret.end(), make_pair(p.first, p.second->default_params));
    return ret;
  }

  // Pins this instance by keeping an extra instance and a ref count.
  // An instance can be pinned multiple times. Instance will be deallocated if
  // no other proxies are left and unpin is called the same number of times
  // pin is called on the same instance.
  static void pin(const shared_proxy& proxy) {
    std::lock_guard<mutex_type> l(mutex());
    ++pin_map()[proxy];
  }

  // "Undo" one pin call on this instance. If all pin calls are "undone" and
  // there are no other proxies to the instance, it will be deallocated.
  static void unpin(const shared_proxy& proxy) {
    std::lock_guard<mutex_type> l(mutex());
    auto it = pin_map().find(proxy);
    assert(it != pin_map().end()); // "Trying to unpin a proxy that was not pinned."
    if (!--it->second) pin_map().erase(it);
  }

 protected:
  static std::map<shared_proxy, uint64_t>& pin_map() {
    static std::map<shared_proxy, uint64_t> data;
    return data;
  }

  // Create a shared_proxy to the requested shared instance. Creates the
  // instance if it is not already present. Once an instance is already created
  // and at least one share_ptr to it is alive, creating additional
  // shared_proxies is pretty cheap.
  static shared_proxy make_shared_from_tuple(const Key& k, const params_type& params) {
    std::lock_guard<mutex_type> l(mutex());
    auto it = registry().find(k);
    // Do we know how to create the requested instance?
    if (it == registry().end()) return nullptr;

    auto jt = it->second->instances.find(params);
    if (jt == it->second->instances.end()) {
      typename registry_entry::instance_entry ie{std::weak_ptr<const T>(), false};
      jt = it->second->instances.insert(make_pair(params, ie)).first;
    }
    // If instance is already being created (probably by another thread), wait.
    while (jt->second.initializing) {
      mutex().unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      mutex().lock();
    }
    // Attempt to create an instance via the weak_ptr.
    shared_proxy ret = jt->second.wptr.lock();
    if (!ret.get()) {
      // Need to create a new instance. Rather than blocking the whole registry
      // class during a potentially expensive allocation, we mark the instance
      // dirty and temporarily release the mutex during the initialization.
      jt->second.initializing = true;
      auto f = it->second->creator.get();
      mutex().unlock();
      ret.reset(tuple_util::tuple_apply(
          std::function<T*(Params...)>([f](Params... p){ return f->operator()(p...); }),
          params));
      mutex().lock();
      jt->second.initializing = false;
      jt->second.wptr = ret;
    }
    return ret;
  }

  static shared_proxy make_updated_from_tuple(const Key& k, const params_type& params) {
    std::lock_guard<mutex_type> l(mutex());
    auto it = registry().find(k);
    if (it == registry().end()) return nullptr;

    auto jt = it->second->instances.find(params);
    if (jt == it->second->instances.end()) {
      mutex().unlock();
      return make_shared_from_tuple(k, params);
    }

    // Unlock mutex to create the new instance.
    mutex().unlock();
    auto old_proxy = make_shared_from_tuple(k, params);
    shared_proxy proxy(make_unique_from_tuple(k, params).release());
    mutex().lock();

    // Recheck again in case another thread modified the data structure.
    // Attempt to replace the existing weak_ptr with the new instance.
    jt = it->second->instances.find(params);
    if (jt == it->second->instances.end()) {
      mutex().unlock();
      return old_proxy;
    }
    if (pin_map().find(old_proxy) != pin_map().end()) {
      pin_map().erase(old_proxy);
      pin(proxy);
    }
    jt->second.wptr = proxy;
    return proxy;
  }

  // Create a user owned, unshared instance.
  static unique_proxy make_unique_from_tuple(const Key& k, const params_type& params) {
    mutex().lock();
    auto it = registry().find(k);
    if (it == registry().end()) return nullptr;
    auto f = it->second->creator.get();
    // Release the mutex before initialization.
    mutex().unlock();
    unique_proxy ret(tuple_util::tuple_apply(
        std::function<T*(Params...)>([f](Params... p){ return f->operator()(p...); }),
        params));
    return ret;
  }

  static registry_type& registry() { static registry_type _; return _; }
  static mutex_type&    mutex()    { static mutex_type _; return _; }
};

// Utility class to create a factory for the given type and return a proxy
// for it as and when required.
template<class T, class Key = std::string, class... Params>
class LazyFactory : public Factory<T, Key, Params...> {
  typedef Factory<T, Key, Params...> super;

 public:
  using Factory<T, Key, Params...>::make_shared;
  using Factory<T, Key, Params...>::make_unique;

  template<class Functor>
  static typename super::shared_proxy make_shared(const Key& k, Params... p,
                                                  Functor f) {
    typename super::shared_proxy proxy = super::make_shared(k, p...);
    if (proxy == nullptr) {
      super::bind(k, p..., f);
      proxy = super::make_shared(k, p...);
    }
    return proxy;
  }

  template<class Functor>
  static typename super::unique_proxy make_unique(const Key& k, Params... p,
                                                  Functor f) {
    typename super::shared_proxy proxy = super::make_unique(k, p...);
    if (proxy == nullptr) {
      super::bind(k, p..., f);
      proxy = super::make_unique(k, p...);
    }
    return proxy;
  }
};

#endif  // _PUBLIC_UTIL_FACTORY_FACTORY_H_
