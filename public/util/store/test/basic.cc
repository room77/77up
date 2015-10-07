// Copyright 2013 B. Uygar Oztekin

// This library tests basic functionality including:
// - Create the store.
// - find()
// - checking an iterator against end();

#include "util/store/test/test_macros.h"

namespace store {
namespace test {
namespace {

template<class Store>
struct BasicFunctionality : public Tester<Store> {
  bool operator()(const std::string& id) const {
    using namespace std;
    auto st = Store::make_shared(id);
    ASSERT(st.get());

    ASSERT(st->end() == st->end());
    ASSERT(st->end().error() == false);
    ASSERT(st->end().supported());

    ASSERT(st->find(this->InvalidKey()) == st->end());
    ASSERT(st->find(this->InvalidKey()).error() == false);
    ASSERT(st->find(this->InvalidKey()).supported());

    for (int i = 0; i < this->size(); ++i) {
      auto kv = this->KeyValue(i);
      auto it = st->find(kv.first);
      ASSERT(it != st->end());
      ASSERT(it->first == kv.first);
      ASSERT(it->second == kv.second);
      ASSERT(this->Validate(*it));
    }
    return true;
  }
};

auto reg_bf_string = Tester<Store<>>::bind("basic functionality",
    []{ return new BasicFunctionality<Store<>>; });

auto reg_bf_custom = Tester<Store<Key, Data>>::bind("basic functionality",
    []{ return new BasicFunctionality<Store<Key, Data>>; });

}
}
}
