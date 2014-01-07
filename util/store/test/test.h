// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_TEST_TEST_H_
#define _PUBLIC_UTIL_STORE_TEST_TEST_H_

#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include "../store.h"

// Utility class to help test stores.
// Tester is templatized to be able to add support for arbitrary keys / values.
// By default key value generators for string -> string and
// test::Key -> test::Data are supported.

namespace store {
namespace test {

// Note that we pick a key type that is not trivial, but at the same time, would
// still preserve sortedness criteria if the underlying store supports it.
// Note that this may not be the case for arbitrary key types including simple
// types such as ints, floats etc.
typedef std::string Key;
typedef std::pair<int, std::string> Data;

template <class Store>
struct Tester : public Factory<Tester<Store>> {

  // Test this store with registered tests.
  static int Test(const std::string& id) {
    using namespace std;
    int failed = 0;
    int count = 0;
    cout << "Testing " << id << endl;
    std::vector<std::string> keys;
    Factory<Tester<Store>>::append_keys(keys);
    for (auto k : keys) {
      auto test = Tester::make_shared(k);
      assert(test.get());
      cout << "[....] " << k;
      ++count;
      auto t0 = chrono::high_resolution_clock::now();
      bool success = test->operator()(id);
      auto t1 = chrono::high_resolution_clock::now();
      auto duration = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
      if (!success) {
        ++failed;
        cout << "\r[fail] " << k << " " << duration << "ms" << endl;
      }
      else {
        cout << "\r[pass] " << k << " " << duration << "ms" << endl;
      }
    }
    cout << count - failed << "/" << count << " tests passed\n" << endl;
    return failed;
  }

  static int Test(const std::vector<std::string>& ids) {
    int failed = 0;
    for (auto& id : ids) failed += Test(id);
    return failed;
  }

  // Derived test classes may use the following as building blocks.

  // Can be used to change number of keys used by tests. Must be modified before
  // anything else. Slower stores may want to use smaller numbers.
  static int& size() { static int n = 10000; return n; }

  // Return the ith key and its corresponding data. End-user may assume that:
  // - Key / data generation is deterministic.
  // - Integer i and key returned has one to one mapping.
  // - Key and data for the key has one to one mapping.
  // - Valid keys are from 0 to num_keys.
  static typename Store::value_type KeyValue(int i);

  // Generate a key that is not "valid" i.e. not in the range 0 to NumKeys()-1.
  static typename Store::key_type InvalidKey() { return KeyValue(-1).first; }

  // Validate the key / value.
  static bool Validate(const typename Store::value_type& kv);


  // Populate the mutable store with size() entries (from 0 to size()-1).
  // Return number of keys that were successfully inserted.
  static int Populate(Store& store) {
    int n = 0;
    for (int i = 0; i < size(); ++i) n += store.insert(KeyValue(i));
    return n;
  }

  // Removes the above populated keys.
  // Return number of keys that were successfully removed.
  static int Unpopulate(Store& store) {
    int n = 0;
    for (int i = 0; i < size(); ++i) n += store.erase(KeyValue(i).first);
    return n;
  }

  // Derived classes should override the following method and test the
  // functionality.
  virtual bool operator()(const std::string& id) const { return false; }
};

}
}

#endif  // _PUBLIC_UTIL_STORE_TEST_TEST_H_
