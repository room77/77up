// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Util function for serialization.

#ifndef _PUBLIC_UTIL_SERIAL_UTILS_SERIALIZER_UTIL_H_
#define _PUBLIC_UTIL_SERIAL_UTILS_SERIALIZER_UTIL_H_

#include <unordered_set>
#include <utility>

#include "base/common.h"
#include "util/string/strutil.h"
#include "util/templates/container_util.h"
#include "util/templates/sfinae.h"

namespace serial {

// This allows two types of default initialization for unspecified fields in
// serialized input.
//
//  sample usage:
//    SERIALIZE(DEFAULT_ZERO | a | b | c)
//               -- sets missing fields to zero or empty string or empty array
//
//    SERIALIZE(DEFAULT_CUSTOM | a | b | c)
//               -- rely on constructor to set missing fields
struct SerializationHelper {};

// Specifies which fields should be chose for serialization.
enum {
  kSerializeModifiedFields = 0,
  kSerializeNonZeroFields = 1,
  kSerializeAllFields = 2
};

// Parameters for binary serialization.
struct BinarySerializationParams {
  BinarySerializationParams(bool raw_binary = false,
                            int serialize_method = kSerializeModifiedFields)
      : raw_binary(raw_binary), serialize_method(serialize_method) {}

  // Whether to serialize as default binary or raw binary.
  bool raw_binary;
  int serialize_method;
};

// Parameters for binary deserialization.
struct BinaryDeSerializationParams {
  BinaryDeSerializationParams(bool raw_binary = false, string* err = nullptr)
      : raw_binary(raw_binary), err(err) {}

  bool raw_binary;
  string* err;
};

// Parameters for JSON serialization.
struct JSONSerializationParams {
  JSONSerializationParams(int indent_increment = 0, int indent = 0,
                          int serialize_method = kSerializeModifiedFields)
      : indent_increment(indent_increment), indent(indent),
        serialize_method(serialize_method) {}

  void Indent() { indent += indent_increment; }
  void Dedent() { indent -= indent_increment; }

  // Whether to serialize as default binary or raw binary.

  int indent_increment;
  int indent;
  int serialize_method;
};

// Parameters for JSON deserialization.
struct JSONDeSerializationParams {
  JSONDeSerializationParams(string* err = nullptr): err(err) {}  // NOLINT
  string* err;
};

// Returns true of the tag is a specialization flag.
inline bool SpecialSerializationFlag(const string& s) {
  static unordered_set<string> kSpecialFlags = {"DEFAULT_ZERO", "DEFAULT_CUSTOM",
                                                "SERIALIZE_ALWAYS",
                                                "SERIALIZE_MODIFIED"};
  return ::util::tl::FindOrNull(kSpecialFlags, s) != nullptr;
}

// Returns the field id and the size type from the serialized id.
inline pair<size_t, int> SerializedIdToIdAndSize(size_t serial_id) {
  return pair<size_t, int>(serial_id >> 3, serial_id & 7);
}

// Returns the serialized id from field id and size type.
inline size_t IdAndSizeToSerializedId(size_t id, int size_type) {
  return (id << 3)  | (size_type & 7);
}

}  // namespace serial

extern serial::SerializationHelper DEFAULT_ZERO;
extern serial::SerializationHelper DEFAULT_CUSTOM;
extern serial::SerializationHelper SERIALIZE_ALWAYS;
extern serial::SerializationHelper SERIALIZE_MODIFIED;
extern serial::SerializationHelper SERIALIZE_TYPEINFO;
extern serial::SerializationHelper SERIALIZE_REQUIRED;
extern serial::SerializationHelper SERIALIZE_OPTIONAL;

#endif  // _PUBLIC_UTIL_SERIAL_UTILS_SERIALIZER_UTIL_H_
