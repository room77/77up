// Copyright 2013 B. Uygar Oztekin

// This library tests forward iteration functionality.

#include "util/store/test/test_macros.h"

namespace store {
namespace test {
namespace {

template<class Store>
struct ForwardIteration : public Tester<Store> {
  bool operator()(const std::string& id) const {
    using namespace std;
    auto st = Store::make_shared(id);
    ASSERT(st.get());

    ASSERT(st->begin().supported());

    int count = 0;
    for (auto& p : *st) {
      ++count;
      ASSERT(this->Validate(p));
    }
    ASSERT(count == this->size());

    count = 0;
    for (auto it = st->begin(); it != st->end(); ++it) {
      ++count;
      ASSERT(this->Validate(*it));
    }
    ASSERT(count == this->size());

    return true;
  }
};

auto reg_fi_string = Tester<Store<>>::bind("forward iteration",
    []{ return new ForwardIteration<Store<>>; });

auto reg_fi_custom = Tester<Store<Key, Data>>::bind("forward iteration",
    []{ return new ForwardIteration<Store<Key, Data>>; });

}
}
}
