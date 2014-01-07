// Copyright 2013 Room77, Inc.
// Author: B. Uygar Oztekin

// Varint encoding support for basic integral types.
// By default unsinged types are encoded in varint and signed ones are encoded
// using zigzag encoding.

#ifndef _PUBLIC_UTIL_SERIAL_TYPES_VARINT_H_
#define _PUBLIC_UTIL_SERIAL_TYPES_VARINT_H_

#include <istream>
#include <ostream>
#include <type_traits>

#include "util/serial/encoding/endian.h"

namespace std {
template<> struct make_unsigned<bool> { typedef uint8_t type; };
}  // namespace

template<class T, bool fixed = false, class D = void> class varint;

// Partial specialization for unsigned types.
template<class T>
class varint<T, false, typename enable_if<!is_signed<T>::value>::type> {
 public:
  typedef T type;
  typedef typename make_unsigned<T>::type unsigned_type;

  varint(T v = T()) : v_(v) {}
  operator const T&() const { return v_; }

  void ToBinary(ostream& out) const {
    unsigned_type n = v_;
    for (; n >> 7; n >>= 7) {
      unsigned char c = (n & 127) | 128;
      out.put(c);
    }
    unsigned char c = n & 127;
    out.put(c);
  }

  bool FromBinary(istream& in) {
    unsigned_type n = T();
    for (int i = 0; true; ++i) {
      unsigned char c = in.get();
      if (in.fail()) return false;
      n |= (static_cast<unsigned_type>(c) & 127) << i * 7;
      if (!(c & 128)) break;
    }
    v_ = n;
    return !in.fail();
  }

  void ToJSON(ostream& out) const {
    out << v_;
  }

  bool FromJSON(istream& in) {
    in >> v_;
    return !in.fail();
  }

 private:
  T v_;
};

// Partial specialization for signed types.
template<class T>
class varint<T, false, typename enable_if<is_signed<T>::value>::type> {
 public:
  typedef T type;
  typedef typename make_unsigned<T>::type unsigned_type;

  varint(T v = T()) : v_(v) {}
  operator const T&() const { return v_; }

  void ToBinary(ostream& out) const {
    unsigned_type n = (v_ << 1) ^ (v_ >> (sizeof(T) * 8 - 1));
    for (; n >> 7; n >>= 7) {
      unsigned char c = (n & 127) | 128;
      out.put(c);
    }
    unsigned char c = n & 127;
    out.put(c);
  }

  bool FromBinary(istream& in) {
    unsigned_type n = T();
    for (int i = 0; !in.fail(); ++i) {
      unsigned char c = in.get();
      if (in.fail()) return false;
      n |= (static_cast<unsigned_type>(c) & 127) << i * 7;
      if (!(c & 128)) break;
    }
    v_ = (n >> 1) ^ (-(n & 1));
    return !in.fail();
  }

  void ToJSON(ostream& out) const {
    out << v_;
  }

  bool FromJSON(istream& in) {
    in >> v_;
    return !in.fail();
  }

 private:
  T v_;
};

// Partial specialization for fixed length types (signed or unsigned).
template<class T>
class varint<T, true> {
 public:
  typedef T type;

  varint(T v = T()) : v_(v) {}
  operator const T&() const { return v_; }

  void ToBinary(ostream& out) const {
    T temp = v_;
    // Always serialize in little endian format.
    serial::endian::HToLE(&temp);
    out.write(reinterpret_cast<char*>(&temp), sizeof(T));
  }

  bool FromBinary(istream& in) {
    in.read(reinterpret_cast<char*>(&v_), sizeof(T));
    if (!in.fail()) {
      // Deserialize from Little endian to Host.
      serial::endian::LEToH(&v_);
      return true;
    }
    return false;
  }

  void ToJSON(ostream& out) const {
    out << v_;
  }

  bool FromJSON(istream& in) {
    in >> v_;
    return !in.fail();
  }

 private:
  T v_;
};

// Alias for fixedint.
template<class T>
using fixedint = varint<T, true>;

#endif  // _PUBLIC_UTIL_SERIAL_TYPES_VARINT_H_
