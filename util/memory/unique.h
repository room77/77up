// Copyright 2010-2012 B. Uygar Oztekin

// Adapted from Uygar's unique storage library.

// This class is a space saving, near-perfect-wrapper around an arbitrary type.
// Let's say, you want to store "stars" information for many hotels, and
// everywhere in your code you used floats. Let's say possible stars values are
// 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5 and 5 (11 unique values). In order to
// store the information you don't want to waste sizeof(float) bytes, but you
// still want the convenience of floats (rather than coming up with a custom
// compression / conversion scheme). You could do:

// util::unique<float, char> stars;

// This stores every unique floating point value ever assigned to a variable of
// type util::unique<float, char> once, and internally indexes using a char. If
// you have many instances of stars information. Other than one time cost of
// storing the unique instances, sizeof(util::unique<float, char>) is 1 byte and
// it acts pretty much like a float.

// Another example. Let's say we want to store city and country information
// in our hotel database loaded in memory. Many hotels have the same city or
// country. Rather than storing the full strings, you could do:

// util::unique<string, unsigned short int> country;

// This assumes that unsigned short int has enough address space to store every
// instance you will use. Using this approach, the average cost of a "string"
// (assuming that you have many of the same strings) becomes closer to
// sizeof(unsigned short int) once you store the unique instance (which is done
// transparently behind the scenes in a thread safe manner). If the number of
// unique countries is less than 256, you could use an unsigned char instead.

// If you attempt to store more unique items then the available address space,
// you will get an assertion failure. It won't transparently fail without you
// noticing. You can switch to a larger address space and recompile.

// With this approach you are trading off speed for memory. Internally,
// creating an instance requires an unordered_map lookup, once created, const
// accesses are relatively cheap, they just use a deque index lookup. Mutable
// accesses need to go through a delegate object. If you don't need to modify
// the object it is best to operate on const references for efficiency. The
// class is thread safe which also introduces some synchronization overhead. If
// speed is critical, benchmark it first and understand pros and cons.

// The instance will behave as if it was the actual type it was wrapping in most
// contexts via operator overloading, implicit typecasts and smart delegate
// classes. Note that it is impossible to write a perfect reference based
// wrapper in C++, so in some cases you would have to call operator() to access
// the wrapped object or its methods. For example:

// util::unique<string> us = "Hello";
// us += " World!";       // us becomes "Hello World!";
// cout << us << endl;    // fine
// cout << us.method();   // ERROR (unless method is explicitly "forwarded").
// cout << us().method(); // This should work.

// This is because C++ comittee decided not to make operator . overloadable.

// Rule of thumb: if it compiles with T, but not util::unique<T>, add ().

// Recommended usage is via the macros:
// UNIQUE8, UNIQUE16, UNIQUE32.

// Examples:
// UNIQUE8(float) stars;  // Independent storage float, 8 bits address space.
// UNIQUE16(int) ints;    // Independent storage int, 16 bits address space.
// UNIQUE32(string) name; // Shared storage string, 32 bits address space.

// Since 8 and 16 bits are relatively small to count unique items, the above
// macros are defined in a way that multiple definitions using the same template
// parameters have independent storage. 32 bits is sufficiently large to "count"
// most stuff we have, so multiple variables of type UNIQUE32(string) will share
// the same storage (same unordered map and deque).

// If for any reason you need to have independent 32 bits storage you can do:
// ::util::unique<type_you_want, uint32_t, UNIQUE_INSTANCE> var1;
// ::util::unique<type_you_want, uint32_t, UNIQUE_INSTANCE> var2;

// util::unique supports Room 77 type serialization.

// See unique_test.cc for some usage examples.

#ifndef _PUBLIC_UTIL_MEMORY_UNIQUE_H_
#define _PUBLIC_UTIL_MEMORY_UNIQUE_H_

#include <cassert>
#include <deque>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef R77_USE_SERIALIZER
#include "util/serial/serializer.h"
#endif

namespace util {

namespace {
  static constexpr unsigned long long basis = 14695981039346656037ULL;
  static constexpr unsigned long long prime = 1099511628211ULL;

  // compile-time hash helper function
  constexpr unsigned long long hash_one(char c, const char* remain,
                                        unsigned long long value) {
    return c == 0 ? value : hash_one(remain[0], remain + 1,
                                     (value ^ c) * prime);
  }
}

// compile-time hash
constexpr unsigned long long static_hash(const char* str) {
  return hash_one(str[0], str + 1, basis);
}

#define UNIQUE_INSTANCE ::util::static_hash(__FILE__) % 1000000 + __COUNTER__
#define SHARED_INSTANCE 0

#define UNIQUE8(T)  ::util::unique<T, uint8_t,  UNIQUE_INSTANCE>
#define UNIQUE16(T) ::util::unique<T, uint16_t, UNIQUE_INSTANCE>
#define UNIQUE32(T) ::util::unique<T, uint32_t>

// Delegate class is specialized via a bool. If we have a class, we just inherit
// from the class for maximum compatibility. Unfortunately it is not possible to
// inherit from internal types in C++ (another annoying oversight). So for
// internal types like int etc., we use a private member variable and overload
// cast operator for the wrapped type.
template<class U, bool is_class>
struct delegate { };

template<class U>
struct delegate<U, true> : public U::value_type {
  delegate(U& u) : U::value_type(u), instance_(u) { }
  ~delegate()      { instance_ = typename U::value_type(*this); }
  typename U::value_type& get()       { return *this; }
  U& instance_;
};

template<class U>
struct delegate<U, false> {
  typedef typename U::value_type value_type;
  delegate(U& u) : instance_(u), value_(u)  { }
  ~delegate()      { instance_ = value_; }
  operator value_type&()  { return value_; }
  value_type& get()       { return value_; }
 private:
  U& instance_;
  typename U::value_type value_;
};

// This is what end users would typically use.
// Template parameters:
// T: type you want to wrap.
// Key: index (or key) type. Must be an integral type and must have enough
// address space to index all possible unique instances you plan to store.
// S: storage class. by default we use indexed_unique_storage, but you can
// implement your own (e.g. pointer or reference based).
template<class T, class Key = uint32_t, int Id = SHARED_INSTANCE>
class unique {
 public:
  // Helper storage class for unique storage. Instances are stored in a deque,
  // and the index of the instance, the "key" is stored in an unordered map from
  // actual type to the key type.
  struct indexed_storage {
    static Key put(const T& v) {
      std::lock_guard<std::mutex> l(mutex());
      auto it = index().find(v);
      if (it == index().end()) {
        assert(storage().size() <= std::numeric_limits<Key>::max());
        it = index().insert(std::pair<T, Key>(v, storage().size())).first;
        storage().push_back(v);
      }
      return it->second;
    }

    static const T& get(Key k) {
      std::lock_guard<std::mutex> l(mutex());
      return storage()[k];
    }

    static std::pair<Key, bool> find(const T& v) {
      std::lock_guard<std::mutex> l(mutex());
      auto it = index().find(v);
      if (it != index().end())
        return std::make_pair(it->second, true);

      return std::make_pair(-1, false);
    }

   private:
    static std::unordered_map<T, Key>& index() {
      static std::unordered_map<T, Key> i;
      return i;
    }
    static std::deque<T>& storage() { static std::deque<T> s; return s; }
    static std::mutex& mutex()      { static std::mutex m; return m; }
  };

  typedef indexed_storage storage_type;
  typedef T value_type;
  typedef unique<T, Key, Id> self;

  unique(const T& t = T()) : key_(storage_type::put(t)) {}
  self& operator=(const unique<T, Key, Id>& t) { key_ = t.key_; return *this; }
  self& operator=(const T& t) { key_ = storage_type::put(t); return *this; }

  // Returns the unique key for the element.
  const Key& key() const {return key_;}

  // Const and mutable references for convenience.
  const T& cref() const { return storage_type::get(key_); }
  delegate<self, std::is_class<T>::value> mref() {
    return delegate<self, std::is_class<T>::value>(*this);
  }

  // End users would typically use implicit cast, get(), or operator().
  operator const T&() const { return cref(); }
  const T& get() const      { return cref(); }
  const T& operator()() const  { return cref(); }
  delegate<self, std::is_class<T>::value> get() { return mref(); }
  delegate<self, std::is_class<T>::value> operator()() { return mref(); }

  // Common comparison operators.
  bool operator !() const { return !cref(); }
  bool operator == (const unique<T, Key, Id>& rhs) const { return cref() == rhs(); }
  bool operator != (const unique<T, Key, Id>& rhs) const { return cref() != rhs(); }
  bool operator < (const unique<T, Key, Id>& rhs) const { return cref() <  rhs(); }
  bool operator > (const unique<T, Key, Id>& rhs) const { return cref() >  rhs(); }
  bool operator <= (const unique<T, Key, Id>& rhs) const { return cref() <= rhs(); }
  bool operator >= (const unique<T, Key, Id>& rhs) const { return cref() >= rhs(); }

  bool operator == (const T& rhs) const { return cref() == rhs; }
  bool operator != (const T& rhs) const { return cref() != rhs; }
  bool operator < (const T& rhs) const { return cref() <  rhs; }
  bool operator > (const T& rhs) const { return cref() >  rhs; }
  bool operator <= (const T& rhs) const { return cref() <= rhs; }
  bool operator >= (const T& rhs) const { return cref() >= rhs; }

  // Common operators T may support.
  template<class R> T& operator+=(const R& rhs) { return mref() += rhs; }
  template<class R> T& operator-=(const R& rhs) { return mref() -= rhs; }
  template<class R> T& operator*=(const R& rhs) { return mref() *= rhs; }
  template<class R> T& operator/=(const R& rhs) { return mref() /= rhs; }
  template<class R> value_type operator+(const R& rhs) { return cref() + rhs; }
  template<class R> value_type operator-(const R& rhs) { return cref() - rhs; }
  template<class R> value_type operator*(const R& rhs) { return cref() * rhs; }
  template<class R> value_type operator/(const R& rhs) { return cref() / rhs; }

  // Support some common container or string interface as well.
  bool empty() const        { return cref().empty(); }
  bool size() const         { return cref().size(); }
  const char* c_str() const { return cref().c_str(); }

#ifdef R77_USE_SERIALIZER
  // Serialization Methods.
  // TODO(pramodg): Implement these methods.
  void ToBinary(std::ostream& out) const {
    ::serial::Serializer::ToBinary(out, get());
  }

  bool FromBinary(std::istream& in) {
    T temp;
    if (!::serial::Serializer::FromBinary(in, &temp)) return false;
    operator=(temp);
    return true;
  }

  void ToJSON(std::ostream& out) const {
    ::serial::Serializer::ToJSON(out, get());
  }

  bool FromJSON(std::istream& in) {
    T temp;
    if (!::serial::Serializer::FromJSON(in, &temp)) return false;
    operator=(temp);
    return true;
  }
#endif

 protected:
  Key key_;
};

// Overload stream operators.
template<class T, class Key, int Id>
std::ostream& operator<<(std::ostream& os, const unique<T, Key, Id>& v) {
  os << v.get();
  return os;
}

template<class T, class Key, int Id>
std::istream& operator>>(std::istream& is, unique<T, Key, Id>& v) {
  T tmp = v;
  is >> tmp;
  v = tmp;
  return is;
}

}

#endif
