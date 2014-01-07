// Copyright 2013 B. Uygar Oztekin

#include <map>
#include "store_converter.h"
#include "../stl/stl_store.h"
#include "../test/test.h"

using namespace store;
using namespace store::test;

auto reg_stl_store = Store<>::bind("low_level", [](){
  return new StlStore<std::map<string, string>>();
});

auto reg_converter_store = Store<Key, Data>::bind("converter_store", [](){
  // In this case, we now that the converter for keys preserves ordering and we
  // would like to test sorted iteration and lower_bound / uppoer_bound.
  // Let's say that this converter preserves key ordering so that the converter
  // would not disable lower / upper bound.
  auto ptr = new StoreConverter<Key, Data>("low_level");
  Tester<Store<Key, Data>>::Populate(*ptr);
  return ptr;
});

int main(int argc, char** argv) {
  return Tester<Store<Key, Data>>::Test("converter_store");
}
