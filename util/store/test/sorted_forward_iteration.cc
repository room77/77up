// Copyright 2013 B. Uygar Oztekin

// Tests forward iteration functionality for stores with sorted keys.

#include "util/store/test/test_macros.h"

namespace store {
namespace test {
namespace {

template<class Store>
struct SortedForwardIteration : public Tester<Store> {
  bool operator()(const std::string& id) const {
    using namespace std;
    auto st = Store::make_shared(id);
    ASSERT(st.get());

    ASSERT(st->begin().supported());

    int count = 0;
    for (auto& p : *st) {
      auto kv = this->KeyValue(count);
      ASSERT(this->Validate(p));
      ASSERT(p.first == kv.first);
      ASSERT(p.second == kv.second);
      ++count;
    }
    ASSERT(count == this->size());

    count = 0;
    for (auto it = st->begin(); it != st->end(); ++it) {
      ASSERT(this->Validate(*it));
      ++count;
    }
    ASSERT(count == this->size());

    return true;
  }
};

auto reg_sfi_string = Tester<Store<>>::bind("sorted forward iteration",
    []{ return new SortedForwardIteration<Store<>>; });

auto reg_sfi_custom = Tester<Store<Key, Data>>::bind("sorted forward iteration",
    []{ return new SortedForwardIteration<Store<Key, Data>>; });

}
}
}
