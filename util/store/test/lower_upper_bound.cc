// Copyright 2013 B. Uygar Oztekin

// Test lower_bound / upper_bound functionality.

#include "util/store/test/test_macros.h"

namespace store {
namespace test {
namespace {

template<class Store>
struct LowerUpperBound : public Tester<Store> {
  bool operator()(const std::string& id) const {
    using namespace std;
    auto st = Store::make_shared(id);
    ASSERT(st.get());

    auto min_not_found = this->KeyValue(-1).first;
    ASSERT(st->lower_bound(min_not_found).supported());
    ASSERT(st->upper_bound(min_not_found).supported());

    ASSERT(st->lower_bound(min_not_found) != st->end());
    ASSERT(*st->lower_bound(min_not_found) == this->KeyValue(0));

    ASSERT(st->upper_bound(min_not_found) != st->end());
    ASSERT(*st->upper_bound(min_not_found) == this->KeyValue(0));

    ASSERT(this->size() > 10);

    {
      auto key = this->KeyValue(1).first;

      ASSERT(st->lower_bound(key) != st->end());
      ASSERT(*st->lower_bound(key) == this->KeyValue(1));

      ASSERT(st->upper_bound(key) != st->end());
      ASSERT(*st->upper_bound(key) == this->KeyValue(2));
    }

    {
      int last = this->size() - 1;
      auto key = this->KeyValue(last).first;

      ASSERT(st->lower_bound(key) != st->end());
      ASSERT(*st->lower_bound(key) == this->KeyValue(last));

      ASSERT(st->upper_bound(key) == st->end());
    }

    {
      auto max_not_found = this->KeyValue(this->size()).first;
      ASSERT(st->lower_bound(max_not_found) == st->end());
      ASSERT(st->upper_bound(max_not_found) == st->end());
    }

    return true;
  }
};

auto reg_lub_string = Tester<Store<>>::bind("lower & upper bound",
    []{ return new LowerUpperBound<Store<>>; });

auto reg_lub_custom = Tester<Store<Key, Data>>::bind("lower & upper bound",
    []{ return new LowerUpperBound<Store<Key, Data>>; });

}
}
}
