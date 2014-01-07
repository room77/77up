// Copyright 2013 B. Uygar Oztekin

// Basic mutation tests for mutable stores.

#include "util/store/test/test_macros.h"

namespace store {
namespace test {
namespace {

template<class Store>
struct Mutation : public Tester<Store> {
  bool operator()(const std::string& id) const {
    using namespace std;
    typename Store::mutable_shared_proxy st = Store::make_shared(id);
    ASSERT(st.get());

    auto kv = this->KeyValue(-1);
    st->erase(kv.first);
    st->insert(kv);
    auto it = st->find(kv.first);
    ASSERT(it != st->end());
    ASSERT(*it == kv);
    ASSERT(st->erase(kv.first));
    return true;
  }
};

auto reg_mutation_string = Tester<Store<>>::bind("mutation",
    []{ return new Mutation<Store<>>; });

auto reg_mutation_custom = Tester<Store<Key, Data>>::bind("mutation",
    []{ return new Mutation<Store<Key, Data>>; });

}
}
}
