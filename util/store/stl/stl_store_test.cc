// Copyright 2013 B. Uygar Oztekin

#include <map>
#include <sstream>
#include "stl_store.h"
#include "../test/test.h"

using namespace store::test;

auto reg_stl_store_string = store::Store<>::bind("stl_store_string", [](){
  auto ptr = new store::StlStore<std::map<std::string, std::string>>;
  Tester<store::Store<>>().Populate(*ptr);
  return ptr;
});

auto reg_stl_store_custom = store::Store<Key, Data>::bind("stl_store_custom", [](){
  auto ptr = new store::StlStore<std::map<Key, Data>>;
  Tester<store::Store<Key, Data>>().Populate(*ptr);
  return ptr;
});

int main(int argc, char** argv) {
  int failed = 0;
  failed += Tester<store::Store<>>::Test("stl_store_string");
  failed += Tester<store::Store<Key, Data>>::Test("stl_store_custom");
  return failed;
}
