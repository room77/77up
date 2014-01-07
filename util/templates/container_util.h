// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// General operations for containers.

#ifndef _PUBLIC_UTIL_TEMPLATES_CONTAINER_UTIL_H_
#define _PUBLIC_UTIL_TEMPLATES_CONTAINER_UTIL_H_

#include <utility>

#include "base/common.h"
#include "base/logging.h"
#include "util/templates/sfinae.h"

namespace util {
namespace tl {

template <typename T>
bool Contains(const T& container, const typename T::key_type& key) {
  return container.find(key) != container.end();
}

inline bool Contains(const string& haystack, const string& needle) {
  return haystack.find(needle) != string::npos;
}

template <typename T>
const typename T::value_type& FindWithDefault(const T& container,
    const typename T::key_type& key, const typename T::value_type& def) {
  typename T::const_iterator iter = container.find(key);
  if (iter != container.end()) return *iter;
  return def;
}

template <typename T>
const typename T::mapped_type& FindWithDefault(const T& container,
    const typename T::key_type& key, const typename T::mapped_type& def) {
  typename T::const_iterator iter = container.find(key);
  if (iter != container.end()) return iter->second;
  return def;
}

template <typename T>
const typename T::value_type* FindOrNull(T& container,
                                         const typename T::key_type& key) {
  auto iter = container.find(key);
  if (iter != container.end()) return &(*iter);
  return nullptr;
}

template <typename T>
const typename T::value_type& FindOrDie(const T& container,
                                        const typename T::key_type& key) {
  typename T::const_iterator iter = container.find(key);
  ASSERT(iter != container.end()) << "Key not found.";
  return *iter;
}

template <typename T>
const typename T::mapped_type& FindMappedOrDie(const T& container,
                                               const typename T::key_type& key) {
  typename T::const_iterator iter = container.find(key);
  ASSERT(iter != container.end()) << "Key not found.";
  return iter->second;
}

// Returns the value for the key if exists, else the key is inserted.
// This is typically used for sets.
template <typename T>
const typename T::value_type& FindWithInsert(T& container,
    const typename T::key_type& key) {
  typename T::iterator iter = container.find(key);
  if (iter != container.end()) return *iter;

  container.insert(container.end(), key);
  iter = container.find(key);
  return *iter;
}

// Returns the key-value for the key if exists, else the key-value is inserted.
// This is typically used for maps to get the <key, value> pair.
template <typename T>
const typename T::value_type& FindWithInsert(T& container,
    const typename T::key_type& key, const typename T::value_type& val) {
  typename T::iterator iter = container.find(key);
  if (iter != container.end()) return *iter;

  iter = container.insert(container.end(), val);
  // iter = container.find(key);
  return *iter;
}

// Returns the value for the key if exists, else the key-value is inserted.
// This is typically used for maps to get the value for the key.
template <typename T>
const typename T::mapped_type& FindWithInsert(T& container,
    const typename T::key_type& key, const typename T::mapped_type& val) {
  typename T::iterator iter = container.find(key);
  if (iter != container.end()) return iter->second;

  iter = container.insert(container.end(), std::make_pair(key, val));
  // iter = container.find(key);
  return iter->second;
}

template <typename T>
typename T::value_type& FindWithInsertMutable(T& container,
    const typename T::key_type& key, const typename T::value_type& val) {
  typename T::iterator iter = container.find(key);
  if (iter != container.end()) return *iter;

  iter = container.insert(container.end(), val);
  // iter = container.find(key);
  return *iter;
}

// Returns the value for the key if exists, else the key-value is inserted.
// This is typically used for maps to get the value for the key.
template <typename T>
typename T::mapped_type& FindWithInsertMutable(T& container,
    const typename T::key_type& key, const typename T::mapped_type& val) {
  typename T::iterator iter = container.find(key);
  if (iter != container.end()) return iter->second;

  iter = container.insert(container.end(), std::make_pair(key, val));
  // iter = container.find(key);
  return iter->second;
}

/* Uncomment if needed in future.
namespace internal {

template<typename T>
T get_value(T& val) {
  return val;
}

template<typename F, typename S>
S get_value(const std::pair<F, S>& val) {
  return val.second;
}

template<typename T>
const T& return_value(const T& val, const T& dummy) {
  return val;
}

template<typename F, typename S>
const S& return_value(const std::pair<F, S>& val, const S& dummy) {
  return val.second;
}

}  // namespace internal
*/


/*
  @author Kyle Konrad <kyle@room77.com?

  Many of these function were copied or adapted from
  http://yapb-soc.blogspot.com/2012/12/quick-and-easy-manipulating-c.html
*/

CREATE_MEMBER_FUNC_CHECK(reserve);

// Zip for output containers with reserve()
template<class O, class F, class S, class T,
         class E=std::enable_if<has_member_func_reserve<O>::type> >
O Zip( F&& f, const S& in1, const T& in2) {
  O out;
  out.reserve(in1.size());
  std::transform(std::begin(in1), std::end(in1),
                 std::begin(in2),
                 std::inserter(out, out.end()),
                 std::forward<F>(f));
  return out;
}

// Zip for output containers without reserve()
template<class O, class F, class S, class T>
O Zip( F&& f, const S& in1, const T& in2) {
  O out;
  std::transform(std::begin(in1), std::end(in1),
                 std::begin(in2),
                 std::forward<F>(f));
  return out;
}

// Zip for when the output the same as first input
// This version allows the user to just call Map (with no template params)
template<class F, template<class...>class S, class X,
         template<class...>class T, class Y,
         class Res = typename std::result_of<F(X, Y)>::type>
S<Res> Zip( F&& f, const S<X>& in1, const T<Y> in2 ) {
  return Zip<S<Res> >(std::forward<F>(f), in1, in2);
}

template<class F, template<class...>class S, class X, class Y,
         class Res=typename std::result_of<F(X,Y)>::type>
S<Res> Zip(F&& f, const S<X>& v, const S<Y>& w) {
  S<Res> r;
  r.reserve(v.size());
  std::transform(std::begin(v), std::end(v),
                 std::begin(w),
                 std::back_inserter(r),
                 std::forward<F>(f));
  return r;
}

// Map for output containers with reserve()
template<class O, class F, class I,
         class E=std::enable_if<has_member_func_reserve<O>::type> >
O Map( F&& f, const I& in ) {
  O out;
  out.reserve(in.size());
  std::transform(std::begin(in), std::end(in),
                 std::inserter(out, out.end()),
                 std::forward<F>(f));
  return out;
}

// Map for output containers without reserve()
template<class O, class F, class I>
O Map( F&& f, const I& in ) {
  O out;
  std::transform(std::begin(in), std::end(in),
                 std::inserter(out, out.end()),
                 std::forward<F>(f));
  return out;
}

// Map for when the output the same as input
// This version allows the user to just call Map (with no template params)
template<class F, template<class...>class S, class X,
         class Res = typename std::result_of<F(X)>::type>
S<Res> Map( F&& f, const S<X>& s ) {
  return Map<S<Res> >(std::forward<F>(f), s);
}

// Get keys from an associative container
template<class C, class O=vector<typename C::key_type> >
O GetKeys(const C& c) {
  return Map<O>([&](const typename C::value_type& v) { return v.first; }, c);
}

// Get values from an associative container
template<class C, class O=vector<typename C::mapped_type> >
O GetValues(const C& c) {
  return Map<O>([&](const typename C::value_type& v) { return v.second; }, c);
}

template<class P, class S>
S Filter(const P& p, S s) {
  using F = std::function< bool(typename S::value_type) >;
  s.erase (std::remove_if( std::begin(s), std::end(s), std::not1(F(p)) ),
           std::end(s));
  return s;
}

template<class C>
typename C::value_type Max(const C& c) {
  ASSERT(!c.empty()) << "Max called on empty container";
  return *std::max_element(c.begin(), c.end());
}

template<class C, class F>
typename C::value_type Max(const C& c, F&& f) {
  ASSERT(!c.empty()) << "Max called on empty container";
  return *std::max_element(c.begin(), c.end(), std::forward<F>(f));
}

template<class C>
typename C::value_type Min(const C& c) {
  ASSERT(!c.empty()) << "Min called on empty container";
  return *std::min_element(c.begin(), c.end());
}

template<class C, class F>
typename C::value_type Min(const C& c, F&& f) {
  ASSERT(!c.empty()) << "Min called on empty container";
  return *std::min_element(c.begin(), c.end(), std::forward<F>(f));
}

template<class C>
vector<int> GetSortIndices(const C& c, bool desc=false) {
  if (desc) {
    return GetSortIndicesBy(c.size(), [&](int i, int j) { return c.at(i) > c.at(j); });
  }
  return GetSortIndicesBy(c.size(), [&](int i, int j) { return c.at(i) < c.at(j); });
}

// return a vector<int> of indices, such that out[i] = index of the ith smallest (using comp) element of c
// comp is a binary function that takes two indices of c and returns a boolean
inline vector<int> GetSortIndicesBy(int size, function<bool(int,int)> comp) {
  vector<int> indices(size);
  for (int i = 0 ; i < indices.size() ; ++i) {
    indices[i] = i;
  }
  sort(indices.begin(), indices.end(), comp);
  return indices;
}


template<class C>
vector<int> GetOrdering(const C& c, bool desc=false) {
  if (desc) {
    return GetOrderingBy(c.size(), [&](int i, int j) { return c.at(i) > c.at(j); });
  }
  return GetOrderingBy(c.size(), [&](int i, int j) { return c.at(i) < c.at(j); });
}

// return a vector<int> of indices, such that i is the out[i] = order statistic of c[i]
// I.e., out[i] = position of the ith element of c after ranking
// comp is a binary function that takes two indices of c and returns a boolean
inline vector<int> GetOrderingBy(int size, function<bool(int,int)>&& comp) {
  vector<int> out(size);
  vector<int> indices = GetSortIndicesBy(size, comp); // indices[i] = index of the ith smallest (using comp) element of c
  for (int i = 0 ; i < indices.size() ; ++i) {
    out[indices[i]] = i;
  }
  return out;
}

// return a sorted vector (of pair of const pointers) of the given map
template<class C>
vector<pair<const typename C::key_type *, const typename C::mapped_type *>>
SortByValue(const C& map,
            function<bool (const typename C::mapped_type&,
                           const typename C::mapped_type&)> comp) {

  using PointerPairType = pair<const typename C::key_type *,
                               const typename C::mapped_type *>;

  vector<PointerPairType> v;
  v.reserve(map.size());
  for (const auto& p : map) v.emplace_back(make_pair(&p.first, &p.second));

  sort(v.begin(), v.end(),
      [&] (const PointerPairType& p1, const PointerPairType& p2) {
    return comp(*p1.second, *p2.second);
  });
  return v;
}

// executes the callback in the sorted order over the given map
template<class C>
void ForEachSortedByValue(
    const C& map,
    function<bool (const typename C::mapped_type&,
                   const typename C::mapped_type&)> comp,
    function<void (const typename C::key_type&,
                   const typename C::mapped_type&)> callback) {
  const auto sorted = SortByValue(map, comp);
  for (const auto& p : sorted) callback(*p.first, *p.second);
}

}  // namespace tl
}  // namespace util


#endif  // _PUBLIC_UTIL_TEMPLATES_CONTAINER_UTIL_H_
