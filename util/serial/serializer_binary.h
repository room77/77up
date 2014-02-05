// Copyright 2013 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

//--------------------------------------------------------------------------
// Binary Serialization.
// A serialized field looks as follows:
//  ---------------------
//  | id | size || data |
//  ---------------------
//
// For basic types, the field_id and size are serialized together.
// The LSB 2 bits specify the size (as defined in type_handlers/type_size.h)
// The rest of the bits specify the id of the field.
// If the size type is kSerialTypeSizeUnknown, then the size is serialized
// after the serialized id.
// Typically, all basic type fields will have an overhead of 1 byte and
// all non basic types will have an overhead of 2 bytes.
//
//--------------------------------------------------------------------------

#ifndef _PUBLIC_UTIL_SERIAL_SERIALIZER_BINARY_H_
#define _PUBLIC_UTIL_SERIAL_SERIALIZER_BINARY_H_

#include <istream>
#include <ostream>
#include <sstream>
#include <utility>

#include "base/defs.h"
#include "util/serial/serialization_data.h"
#include "util/serial/type_handlers/deserialization_callback.h"
#include "util/serial/type_handlers/type_size.h"
#include "util/serial/type_handlers/binary_deserialization.h"
#include "util/serial/type_handlers/binary_serialization.h"
#include "util/serial/utils/serializer_util.h"
#include "util/serial/utils/stream_util.h"
#include "util/serial/types/varint.h"

namespace serial {

class SerializeBinary {
  typedef SerializationData::FieldData SDField;

 public:
  explicit SerializeBinary(ostream& out, const SerializationData& data,
                           const BinarySerializationParams& params = BinarySerializationParams())
      : out_(out), data_(data), params_(params), serializer_(params) {}

 public:
  SerializeBinary& BeginIteration() {
    next_field_ = 0;
    return *this;
  }

  SerializeBinary& EndIteration() {
    // Add the reserved tag to signify end of struct.
    size_t id = 0;
    serializer_(out(), id);
    return *this;
  }

  template<typename Field>
  SerializeBinary& operator /(const Field& v) {
    ASSERT_LT(next_field_, data_.name_id_list().size());
    const size_t field_id = data_.name_id_list()[next_field_++].second;
    const SDField* field_data = data_.field_data(field_id);
    ASSERT_NOTNULL(field_data);

    size_t serial_id = IdAndSizeToSerializedId(field_data->id,
                                               SizeByType<Field>::value);

    // Serialize Metadata for the field.
    serializer_(out(), serial_id);

    // If the field has unknown type we need to figure out its size.
    if (SizeByType<Field>::value == kSerialTypeUnknownFixedInt) {
      ostream::pos_type pos = out().tellp();
      fixedint<unsigned int> size = 0;
      // Dump the size first.
      serializer_(out(), size);

      // Serialize the field.
      serializer_(out(), v);

      // Go back and refill the size.
      ostream::pos_type new_pos = out().tellp();
      size = new_pos - pos - sizeof(unsigned int);

      out().seekp(pos);
      serializer_(out(), size);

      // Reset to the end.
      out().seekp(new_pos);
    } else {
      serializer_(out(), v);
    }
    return *this;
  }

  // Ignore the special flags.
  SerializeBinary& operator /(const SerializationHelper& v) {
    return *this;
  }

  // Ignore the * operators.
  template<typename Field>
  SerializeBinary& operator *(const Field& v) { return *this; }

 protected:
  ostream& out() { return out_;}

  ostream& out_;
  const SerializationData& data_;
  BinarySerializationParams params_;
  BinarySerializationByType serializer_;
  // The next field index in the iteration. This will match the indices in the
  // vector names_ exactly.
  size_t next_field_ = 0;
};

class DeSerializeBinary {
  typedef SerializationData::FieldData SDField;

 public:
  explicit DeSerializeBinary(const SerializationData& data,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams())
      : data_(data), params_(params), deserializer_(params) {}

 public:
  bool Deserialize(istream& in, char* res, const string& type_name);

 protected:
  // Skip ahead in the stream when the size in unknown.
  void SkipAheadUnknownId(istream& in, int type);

  void SkipAheadKnownId(istream& in, int type);

  const SerializationData& data_;
  BinaryDeSerializationParams params_;
  BinaryDeSerializationByType deserializer_;
};

}  // namespace serial

// Implementation for Binary serialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define BINARY_SERIALIZATION_IMPL(varlist___, ...)                             \
  __VA_ARGS__ void ToBinaryImpl(ostream& out__,                                \
      const ::serial::BinarySerializationParams& params__) const {             \
    typename ::serial::SerializeBinary r__(out__,                              \
       this->GetSerializationData(), params__);                                \
    r__.BeginIteration() / varlist___;                                         \
    r__.EndIteration();                                                        \
  }                                                                            \

// Implementation for Binary deserialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define BINARY_DESERIALIZATION_IMPL(varlist___, ...)                           \
  __VA_ARGS__ bool FromBinaryImpl(istream& in__,                               \
      const ::serial::BinaryDeSerializationParams& params__) {                 \
    typedef typename ::util::tl::remove_rcv<decltype(*this)>::type __MyType;   \
    typename ::serial::DeSerializeBinary r__(                                  \
        this->GetSerializationData(), params__);                               \
    bool res__ = r__.Deserialize(in__, reinterpret_cast<char*>(this),          \
                                 typeid(__MyType).name());                     \
    if (res__) res__ = ::serial::DeserializationCallbackRunner()(this);        \
    return res__;                                                              \
  }                                                                            \

// SERIALIZATION_BINARY_IMPL defines the functions for Binary
// serialization/deserialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define SERIALIZATION_BINARY_IMPL(varlist___, ...)                             \
  BINARY_SERIALIZATION_IMPL(varlist___, __VA_ARGS__);                          \
                                                                               \
  BINARY_DESERIALIZATION_IMPL(varlist___, __VA_ARGS__)                         \

// Dummy Implementation for Binary serialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define DUMMY_BINARY_SERIALIZATION_IMPL(varlist___, ...)                       \
  __VA_ARGS__ void ToBinaryImpl(ostream& out__,                                \
      const ::serial::BinarySerializationParams& params__) const {}            \

// IDummy mplementation for Binary deserialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define DUMMY_BINARY_DESERIALIZATION_IMPL(varlist___, ...)                     \
  __VA_ARGS__ bool FromBinaryImpl(istream& in__,                               \
      const ::serial::BinaryDeSerializationParams& params__) { return false; } \

// SERIALIZATION_BINARY_DUMMY_IMPL defines the dummy functions for Binary
// serialization/deserialization.
#define SERIALIZATION_BINARY_DUMMY_IMPL(varlist___, ...)                       \
  DUMMY_BINARY_SERIALIZATION_IMPL(varlist___, __VA_ARGS__);                    \
                                                                               \
  DUMMY_BINARY_DESERIALIZATION_IMPL(varlist___, __VA_ARGS__)                   \


#endif  // _PUBLIC_UTIL_SERIAL_SERIALIZER_BINARY_H_
