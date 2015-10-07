// Copyright 2013 B. Uygar Oztekin

// Multithreaded stress test hammering the store with successful and un

#include <random>
#include <future>
#include <deque>
#include "util/store/test/test_macros.h"

namespace store {
namespace test {
namespace {

template<class Store>
struct StressTest : public Tester<Store> {
  bool operator()(const std::string& id) const {

    // We will run this function in a multi-threaded way.
    auto func = [&]() {
      using namespace std;
      auto store = Store::make_shared(id);
      ASSERT(store.get());

      ASSERT(store->end() == store->end());
      ASSERT(store->end().error() == false);
      ASSERT(store->end().supported());

      ASSERT(store->find(this->InvalidKey()) == store->end());
      ASSERT(store->find(this->InvalidKey()).error() == false);
      ASSERT(store->find(this->InvalidKey()).supported());

      vector<int> probabilities;
      for (int i = 0; i < this->size(); ++i) {
        probabilities.push_back(i);
      }
      std::random_device rd;
      std::mt19937 gen(rd());
      std::discrete_distribution<> random(probabilities.begin(), probabilities.end());

      int num_lookups = this->size() * 2;

      // All successful lookups.
      for (int c = 0; c < num_lookups; ++c) {
        int i = random(gen);
        auto kv = this->KeyValue(i);
        auto it = store->find(kv.first);
        ASSERT(it != store->end());
        ASSERT(it->first == kv.first);
        ASSERT(it->second == kv.second);
        ASSERT(this->Validate(*it));
      }

      // Some successful, some unsuccessful lookups.
      for (int c = 0; c < num_lookups; ++c) {
        int i = random(gen) + 5;
        auto kv = this->KeyValue(i);
        auto it = store->find(kv.first);
        ASSERT(i < this->size() ? it != store->end() : it == store->end());
      }

      // All unsuccessful lookups.
      for (int c = 0; c < num_lookups; ++c) {
        int i = random(gen) + this->size();
        auto kv = this->KeyValue(i);
        auto it = store->find(kv.first);
        ASSERT(it == store->end());
      }

      return true;
    };

    // Launch the threads in parallel.
    std::deque<std::future<bool>> results;
    for (int i = 0; i < 4; ++i) results.push_back(std::async(std::launch::async, func));
    for (auto& result : results) if (!result.get()) return false;
    return true;
  }
};

auto reg_bf_string = Tester<Store<>>::bind("stress test",
    []{ return new StressTest<Store<>>; });

auto reg_bf_custom = Tester<Store<Key, Data>>::bind("stress test",
    []{ return new StressTest<Store<Key, Data>>; });

}
}
}
