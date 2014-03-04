// Copyright 2013 B. Uygar Oztekin

#include <map>
#include "simple_cacher.h"
#include "../stl/stl_store.h"
#include "../test/test.h"
#include "../test/basic.h"
#include "../test/stress.h"

using namespace store;
using namespace store::test;

auto reg_stl_store = Store<>::bind("low_level", [](){
  return new StlStore<std::map<std::string, std::string>>();
});

Store<>::shared_proxy LowLevelStore() {
  auto proxy = Store<>::make_shared("low_level");
  assert(proxy.get());
  Tester<Store<>>::Populate(*Store<>::mutable_shared_proxy(proxy));
  return proxy;
}

auto reg_cache_all = Store<>::bind("preload_all", [](){
  return new SimpleCacher<>(LowLevelStore(), true);
});

auto reg_cache_inf = Store<>::bind("cache_inf", [](){
  return new SimpleCacher<>(LowLevelStore(), false);
});

auto reg_cache_1_4 = Store<>::bind("cache_1/4", [](){
  return new SimpleCacher<>(LowLevelStore(), false, Tester<Store<>>::size() / 4);
});

auto reg_cache_1_2 = Store<>::bind("cache_1/2", [](){
  return new SimpleCacher<>(LowLevelStore(), false, Tester<Store<>>::size() / 2);
});

int main(int argc, char** argv) {
  return Tester<Store<>>::Test({"preload_all", "cache_inf", "cache_1/2", "cache_1/4"});
}
