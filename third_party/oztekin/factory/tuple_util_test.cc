// Copyright 2012 B. Uygar Oztekin
// Copied and modified from Uygar's personal libraries.

#include <iostream>
#include <cassert>
#include "tuple_util.h"

using namespace std;

void TestFunction(int i, double d) {
  cout << "Params I got: " << i << ", " << d << endl;
}

template<int> struct int_{}; // compile-time counter

template<class Ch, class Tr, class Tuple, int I>
void print_tuple(std::basic_ostream<Ch,Tr>& os, Tuple const& t, int_<I>) {
  print_tuple(os, t, int_<I-1>());
  os << ", " << std::get<I>(t);
}

template<class Ch, class Tr, class Tuple>
void print_tuple(std::basic_ostream<Ch,Tr>& os, Tuple const& t, int_<0>) {
  os << std::get<0>(t);
}

template<class Ch, class Traits, class... Args>
std::ostream& operator<<(std::basic_ostream<Ch,Traits>& os, std::tuple<Args...> const& t) {
  os << "(";
  print_tuple(os, t, int_<sizeof...(Args)-1>());
  return os << ")";
}


int main() {
  using namespace tuple_util;
  tuple_apply(std::function<void(int, double)>(TestFunction), tuple<int, double>(1, 2.3));
  tuple<int, int, int, int, int> t1(1, 2, 3, 4, 5);
  tuple<int, int, int> t2(10, 20, 30);

  cout << t1 << endl;
  cout << t2 << endl;

  tuple_merge(t1, t2);
  cout << t1 << endl;
  tuple<int, int, int, int, int> expected_t1(10, 20, 30, 4, 5);
  assert(t1 == expected_t1);

  tuple_merge(t1, tuple<int, int, int, int, int>(-1, -2, -3, -4, -5));
  cout << t1 << endl;
  expected_t1 = tuple<int, int, int, int, int>(-1, -2, -3, -4, -5);
  assert(t1 == expected_t1);

  tuple<> empty;
  tuple_merge(t1, empty);
  assert(t1 == expected_t1);

  return 0;
}
