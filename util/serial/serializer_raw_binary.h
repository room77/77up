// Copyright 2013 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

//--------------------------------------------------------------------------
// Raw Binary Serialization.
// A raw serialized field looks as follows:
//  --------
//  | data |
//  --------
// The raw binary serialization is a stripped down, fast version of the default
// binary serialization.
// All fields are serialized on order and they are also deserialized in order.
// It is useful for final structures that would never need to be changed or
// are written in such a way that the serialized data is always for the latest
// structure.
// This does not support any of the smart features offered by default
// binary serialization.
// An added advantage of this serialization is free-form serialization where
// non-struct variables can be serialized/deserialized. Moreover, for
// serialization, rvalues can also be used directly.
//--------------------------------------------------------------------------

#ifndef _PUBLIC_UTIL_SERIAL_SERIALIZER_RAW_BINARY_H_
#define _PUBLIC_UTIL_SERIAL_SERIALIZER_RAW_BINARY_H_

#include <istream>
#include <ostream>
#include <sstream>

#include "base/defs.h"
#include "util/string/strutil.h"
#include "util/serial/type_handlers/deserialization_callback.h"
#include "util/serial/type_handlers/binary_deserialization.h"
#include "util/serial/type_handlers/binary_serialization.h"
#include "util/serial/utils/serializer_util.h"
#include "util/serial/utils/stream_util.h"

namespace serial {

// Raw binary serialization.
class SerializeRawBinary {
 public:
  explicit SerializeRawBinary(ostream& out,
      const BinarySerializationParams& params = BinarySerializationParams())
      : out_(out), params_(params), serializer_(params) {}

  // Note: All these operators will always return current class object.
  // We need to override all these operators in any derived class.
  template<typename Field>
  SerializeRawBinary& operator /(const Field& v) {
    serializer_(out(), v);
    return *this;
  }

  // Ignore the special flags.
  SerializeRawBinary& operator /(const SerializationHelper& v) {
    return *this;
  }

  // Ignore the * operators.
  template<typename Field>
  SerializeRawBinary& operator *(const Field& v) { return *this; }

 protected:
  ostream& out() { return out_;}

  ostream& out_;
  BinarySerializationParams params_;
  BinarySerializationByType serializer_;
};

class DeSerializeRawBinary {
 public:
  explicit DeSerializeRawBinary(istream& in,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams())
      : in_(in), params_(params), deserializer_(params) {
    // Check if we are at end of file.
    in_.peek();
    if (in_.eof()) ok_ = false;
  }

  template<typename Field>
  DeSerializeRawBinary& operator /(Field& v) {  // NOLINT
    if (!ok_) return *this;

    deserializer_(in(), &v);
    if (in().fail()) {
      ok_= false;
      util::LogParsingError(in(), -1, "", params_.err);
    }

    return *this;
  }

  // Ignore the special flags.
  DeSerializeRawBinary& operator /(SerializationHelper& v) {  // NOLINT
    return *this;
  }

  // Ignore the * operators.
  template<typename Field>
  DeSerializeRawBinary& operator *(const Field& v) { return *this; }

  bool ok() const { return ok_; }

 protected:
  istream& in() { return in_;}

  bool ok_ = true;
  istream& in_;
  BinaryDeSerializationParams params_;
  BinaryDeSerializationByType deserializer_;
};

}  // namespace serial


// Implementation for Raw Binary serialization.
#define RAW_BINARY_SERIALIZATION_IMPL(varlist___, ...)                         \
  __VA_ARGS__ void ToRawBinaryImpl(ostream& out__,                             \
      const ::serial::BinarySerializationParams& params__) const {             \
    ::serial::SerializeRawBinary r__(out__, params__);                         \
    r__ / varlist___;                                                          \
  }                                                                            \

// Implementation for Raw Binary deserialization.
#define RAW_BINARY_DESERIALIZATION_IMPL(varlist___, ...)                       \
  __VA_ARGS__ bool FromRawBinaryImpl(istream& in__,                            \
    const ::serial::BinaryDeSerializationParams& params__) {                   \
    ::serial::DeSerializeRawBinary r__(in__, params__);                        \
    r__ / varlist___;                                                          \
    bool res__ = r__.ok();                                                     \
    if (res__) res__ = ::serial::DeserializationCallbackRunner()(this);        \
    return res__;                                                              \
  }                                                                            \

// SERIALIZE_BINARY defines the functions for Binary serialization/deserialization.
#define SERIALIZATION_RAW_BINARY_IMPL(varlist___, ...)                         \
  RAW_BINARY_SERIALIZATION_IMPL(varlist___, __VA_ARGS__);                      \
                                                                               \
  RAW_BINARY_DESERIALIZATION_IMPL(varlist___, __VA_ARGS__);                    \

#endif  // _PUBLIC_UTIL_SERIAL_SERIALIZER_RAW_BINARY_H_
