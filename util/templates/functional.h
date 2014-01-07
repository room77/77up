#ifndef _PUBLIC_UTIL_TEMPLATES_FUNCTIONAL_H_
#define _PUBLIC_UTIL_TEMPLATES_FUNCTIONAL_H_

#include <functional>

#include "util/serial/serializer.h"

namespace util {
namespace tl {

// Binary-signature-based hash function of the object.
template<class T>
struct binary_hash {
  size_t operator()(const T& v) const {
    return hash<string>()(::serial::Serializer::ToRawBinary(v));
  }

  size_t operator()(const T* v) const {
    return operator ()(*v);
  }
};

// Equality comparison function for the object.
template<class T>
struct binary_equal {
  bool operator()(const T& v1, const T& v2) const {
    // Objects are equal if their serialized signatures match.
    return (::serial::Serializer::ToBinary(v1) ==
        ::serial::Serializer::ToBinary(v2));
  }

  bool operator()(const T* v1, const T* v2) const {
    return operator ()(*v1, *v2);
  }
};

template<typename T, typename Member, typename Base, Member Base::*member>
struct GetInBase : std::unary_function <Base, T> {
  Member operator() (const Base& x) const {
    return x.*member;
  }
};

// get a specific member of the struct.
// E.g. get<tStruct, decltype(tStruct::field), &tStruct::field>(my_struct);
template<typename T, typename Member, Member T::*member>
struct Get : GetInBase<T, Member, T, member> {};


// only works if arguments are function objects
// template <typename A, typename B, typename C>
// std::function<C(A)> Compose(std::function<C(B)> f, std::function<B(A)> g) {
//   return [f,g](A x) { return f(g(x)); };
// }

// only works if arguments are function objects
// template <typename F, typename G>
// std::function<typename F::result_type(typename G::argument_type)> Compose(F f, G g) {
//   return [f,g](typename G::argument_type x) { return f(g(x)); };
// }

// nested result_of doesn't seem to work:
//   no known conversion for argument 1 from ‘std::result_of<std::function<int(int)>(int)>’ to ‘int’
// template <typename Arg, typename F, typename G>
// std::function<typename std::result_of<F(
//     typename std::result_of<G(Arg)>)>(Arg)> Compose(F f, G g) {
//   return [f,g](Arg x) { return f(g(x)); };
// }

// TODO(KK): doesn't support functors
template<typename F, typename G>
struct ComposeFunctor
{
  F f_;
  G g_;
  explicit ComposeFunctor(F&& f, G&& g) : f_(f), g_(g) {}

  template<typename Arg>
  auto operator()(Arg arg) const -> decltype(f_(g_(arg))) {
    return f_(g_(arg));
  }
};

// wrap in a function so template deduction works
template<class F, class G>
inline ComposeFunctor<F, G> Compose(F&& f, G&& g) {
  return ComposeFunctor<F, G>(f, g);
}

}  // namespace tl
}  // namespace util

#endif  // _PUBLIC_UTIL_TEMPLATES_FUNCTIONAL_H_
