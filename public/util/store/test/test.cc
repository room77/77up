// Copyright 2013 B. Uygar Oztekin

#include <deque>
#include <sstream>
#include "test.h"

using namespace std;

namespace store {
namespace test {

template<>
Store<string, string>::value_type Tester<Store<string, string>>::KeyValue(int i) {
  stringstream ss;
  ss << setw(5) << setfill('0') << i;
  return make_pair(ss.str(), ss.str() + "val");
}

template<>
bool Tester<Store<string, string>>::Validate(const Store<string, string>::value_type& kv) {
  stringstream ss;
  ss << kv.first;
  int i;
  ss >> i;
  return i >= 0 && i < size() && kv == KeyValue(i);
}

template<>
Store<Key, Data>::value_type
Tester<Store<Key, Data>>::KeyValue(int i) {
  stringstream ss;
  ss << setw(10) << setfill('0') << i << "val";
  return make_pair(ss.str(), make_pair(i, ss.str()));
}

template<>
bool Tester<Store<Key, Data>>::Validate(const Store<Key, Data>::value_type& kv) {
  stringstream ss;
  ss << kv.first;
  int i;
  ss >> i;
  return i >= 0 && i < size() && kv == KeyValue(i);
}

}
}
