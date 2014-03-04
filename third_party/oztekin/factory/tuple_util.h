// Copyright 2012 B. Uygar Oztekin
// Copied and modified from Uygar's personal libraries.

// Utilities to deal with std::tuple.
// Most of the stuff here uses compile time recursions and template meta
// programming. They are expected to be optimized away by the compiler.

// std naming conventions are used for consistency with similar std functions
// (e.g. tuple_cat etc.).

#ifndef _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_TUPLE_UTIL_H_
#define _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_TUPLE_UTIL_H_

#include <tuple>
#include <functional>

using std::tuple;

// Support functions that does not need to be exposed.
namespace {
  //
  // Apply support
  //
  template<int...> struct index_tuple {};
  template<int N, class Index, class... Args> struct make_index_helper;

  template<int N, int... Index, class T, class... Args>
  struct make_index_helper<N, index_tuple<Index...>, T, Args...> {
    typedef typename make_index_helper<N + 1, index_tuple<Index..., N>, Args...>::type type;
  };

  template<int N, int... Index>
  struct make_index_helper<N, index_tuple<Index...> > {
    typedef index_tuple<Index...> type;
  };

  template<typename... Args> struct make_index : make_index_helper<0, index_tuple<>, Args...> {};

  template<class Func, class... Args, int... Index >
  typename Func::result_type apply(Func f, index_tuple< Index... >, tuple<Args...>&& t) {
    return f(std::forward<Args>(std::get<Index>(t))...);
  }

  //
  // Merge support
  //
  template<int...> struct indexer_ {};

  // Set Ith value of T1 to Ith value of t2, and call this recursively.
  // Recursion is at compile time and should be optimized away.
  template<class Tuple1, class Tuple2, int I>
  void merge(Tuple1& t1, const Tuple2& t2, indexer_<I>) {
    std::get<I>(t1) = std::get<I>(t2);
    merge(t1, t2, indexer_<I-1>());
  }

  // End recursion criteria. Process the 0th entry.
  template<class Tuple1, class Tuple2>
  void merge(Tuple1& t1, const Tuple2& t2, indexer_<0>) { std::get<0>(t1) = std::get<0>(t2); }

  // Handle empty t2 case.
  template<class Tuple1, class Tuple2, int I>
  void merge(Tuple1& t1, const Tuple2& t2, indexer_<-1>) { }
} // namespace



//
// End user API starts here.
//
namespace tuple_util {

// If t is an lvalue, we need to use this approach.
template<class Func, class... Args>
typename Func::result_type tuple_apply(Func f, const tuple<Args...>& t) {
  return apply(f, typename make_index<Args...>::type(), tuple<Args...>(t));
}

// Uses forwarding if t is an rvalue (more efficient).
template<class Func, class... Args>
typename Func::result_type tuple_apply(Func f, tuple<Args...>&& t) {
  return apply(f, typename make_index<Args...>::type(), std::forward<tuple<Args...>>(t));
}

// Merging tuples of different sizes but compatible types.
// Tuple2 must be a prefix and subset of Tuple1.
// If we have t1 = (1, 2, 3, 4, 5) and t2 = (10, 20, 30).
// After merge(t1, t2), t1 will contain (10, 20, 30, 4, 5).
template<class Tuple1, class Tuple2>
void tuple_merge(Tuple1& t1, const Tuple2& t2) {
  merge(t1, t2, indexer_<std::tuple_size<Tuple2>::value - 1>());
}

// Specialization for empty t2.
template<class Tuple1>
void tuple_merge(Tuple1& t1, const std::tuple<>& t2) {}

} // namespace tuple_util

#endif  // _PUBLIC_THIRD_PARTY_OZTEKIN_FACTORY_TUPLE_UTIL_H_
