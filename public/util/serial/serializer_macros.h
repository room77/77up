// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines the top level macros for serialization.
// Each macro accepts a list of fields separated using '/'.
// Using '/' we can explicitly associate ids with fields.
// Every field in the list 'must' specify its id using '*' operator.
/*
struct ExampleWithSlashAndStar {
  int a, b, c;
  string d;
  pair< float, vector<unsigned int> > e;
  SERIALIZE(a*1 / b*2 / c*3 / d*4 / e*5);
}
*/

// Note: The operator '%' is currently not used but can be used in future to
// support new features for a given field.

#ifndef _PUBLIC_UTIL_SERIAL_SERIALIZER_MACROS_H_
#define _PUBLIC_UTIL_SERIAL_SERIALIZER_MACROS_H_

#include <istream>
#include <ostream>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "util/serial/serialization_data.h"
#include "util/serial/serializer_binary.h"
#include "util/serial/serializer_csv.h"
#include "util/serial/serializer_json.h"
#include "util/serial/serializer_raw_binary.h"
#include "util/serial/utils/serializer_util.h"
#include "util/templates/container_util.h"
#include "util/templates/type_traits.h"

// ------------------------------------------------------------
// Internal Macros. Please only use the public macros below.
// ------------------------------------------------------------

// Interface for Binary serialization.
#define BINARY_SERIALIZATION_INTERFACE(varlist___) \
  void ToBinary(ostream& out__,  \
                const ::serial::BinarySerializationParams& params__ = \
                    ::serial::BinarySerializationParams()) const { \
    if (params__.raw_binary) ToRawBinaryImpl(out__, params__); \
    else ToBinaryImpl(out__, params__); \
  } \
  \
  string ToBinary(const ::serial::BinarySerializationParams& params__ = \
                      ::serial::BinarySerializationParams()) const { \
    ostringstream ss__; \
    ToBinary(ss__, params__); \
    return ss__.str(); \
  } \
  \
  void ToRawBinary(ostream& out__, \
                   const ::serial::BinarySerializationParams& params__ = \
                       ::serial::BinarySerializationParams()) const { \
    ::serial::BinarySerializationParams local_params__ = params__; \
    local_params__.raw_binary = true; \
    return ToBinary(out__, local_params__); \
  } \
  \
  string ToRawBinary(const ::serial::BinarySerializationParams& params__ = \
                         ::serial::BinarySerializationParams()) const { \
    ::serial::BinarySerializationParams local_params__ = params__; \
    local_params__.raw_binary = true; \
    return ToBinary(local_params__); \
  } \

// Interface for Binary deserialization.
#define BINARY_DESERIALIZATION_INTERFACE(varlist___) \
  bool FromBinary(istream& in__, \
                  const ::serial::BinaryDeSerializationParams& params__ = \
                      ::serial::BinaryDeSerializationParams()) { \
    return params__.raw_binary ? FromRawBinaryImpl(in__, params__) : \
        FromBinaryImpl(in__, params__); \
  } \
  \
  bool FromBinary(const string& s__, \
                  const ::serial::BinaryDeSerializationParams& params__ = \
                      ::serial::BinaryDeSerializationParams()) { \
    istringstream ss(s__); \
    return FromBinary(ss, params__); \
  } \
  \
  bool FromBinary(const char* s__, size_t len__, \
                  const ::serial::BinaryDeSerializationParams& params__ = \
                      ::serial::BinaryDeSerializationParams()) { \
    return FromBinary(string(s__, len__), params__); \
  } \
  bool FromRawBinary(istream& in__, \
                     const ::serial::BinaryDeSerializationParams& params__ = \
                         ::serial::BinaryDeSerializationParams()) { \
    ::serial::BinaryDeSerializationParams local_params__ = params__; \
    local_params__.raw_binary = true; \
    return FromBinary(in__, local_params__); \
  } \
  \
  bool FromRawBinary(const string& s__, \
                     const ::serial::BinaryDeSerializationParams& params__ = \
                         ::serial::BinaryDeSerializationParams()) { \
    ::serial::BinaryDeSerializationParams local_params__ = params__; \
    local_params__.raw_binary = true; \
    return FromBinary(s__, local_params__); \
  } \
  \
  bool FromRawBinary(const char* s__, size_t len__, \
                     const ::serial::BinaryDeSerializationParams& params__ = \
                         ::serial::BinaryDeSerializationParams()) { \
    ::serial::BinaryDeSerializationParams local_params__ = params__; \
    local_params__.raw_binary = true; \
    return FromBinary(s__, len__, local_params__); \
  } \


// Interface for JSON serialization.
#define JSON_SERIALIZATION_INTERFACE(varlist___) \
  void ToJSON(ostream& out__,                                     \
              const ::serial::JSONSerializationParams& params__ = \
                  ::serial::JSONSerializationParams()) const { \
    ToJSONImpl(out__, params__); \
  } \
  \
  string ToJSON(const ::serial::JSONSerializationParams& params__ = \
                    ::serial::JSONSerializationParams()) const { \
    ostringstream ss__; \
    ToJSON(ss__, params__); \
    return ss__.str(); \
  } \

// Interface for Binary deserialization.
#define JSON_DESERIALIZATION_INTERFACE(varlist___) \
  bool FromJSON(istream& in__, \
                const ::serial::JSONDeSerializationParams& params__ = \
                    ::serial::JSONDeSerializationParams()) { \
    return FromJSONImpl(in__, params__); \
  } \
  \
  bool FromJSON(const string& s__, \
                  const ::serial::JSONDeSerializationParams& params__ = \
                      ::serial::JSONDeSerializationParams()) { \
    istringstream ss(s__); \
    return FromJSON(ss, params__); \
  } \
  \
  bool FromJSON(const char* s__, size_t len__, \
                  const ::serial::JSONDeSerializationParams& params__ = \
                      ::serial::JSONDeSerializationParams()) { \
    return FromJSON(string(s__, len__), params__); \
  } \

#define SERIALIZATION_BINARY_INTERFACE(varlist___) \
  BINARY_SERIALIZATION_INTERFACE(varlist___);  \
  \
  BINARY_DESERIALIZATION_INTERFACE(varlist___) \

#define SERIALIZATION_JSON_INTERFACE(varlist___) \
  JSON_SERIALIZATION_INTERFACE(varlist___);  \
  \
  JSON_DESERIALIZATION_INTERFACE(varlist___) \


// ------------------------------------------------------------
// Public Macros.
// ------------------------------------------------------------

// The macros for complete binary serialization.
#define SERIALIZE_BINARY(varlist___) \
  SERIALIZATION_DATA(varlist___);  \
  \
  SERIALIZATION_BINARY_IMPL(varlist___, inline); \
  \
  SERIALIZATION_RAW_BINARY_IMPL(varlist___, inline); \
  \
  SERIALIZATION_BINARY_INTERFACE(varlist___) \

// The macros for raw binary serialization.
#define SERIALIZE_RAW_BINARY(varlist___) \
  SERIALIZATION_BINARY_DUMMY_IMPL(varlist___, inline); \
  \
  SERIALIZATION_RAW_BINARY_IMPL(varlist___, inline); \
  \
  SERIALIZATION_BINARY_INTERFACE(varlist___) \

// The macros for complete JSON serialization.
#define SERIALIZE_JSON(varlist___) \
  SERIALIZATION_DATA(varlist___);  \
  \
  SERIALIZATION_JSON_IMPL(varlist___, inline); \
  \
  SERIALIZATION_JSON_INTERFACE(varlist___) \

// SERIALIZE macro is typically used inside a struct definition to
// spell out all variables to be serialized.
// The macro generates the following functions (XX == Binary, JSON or RawBinary):
//
// Serialization:
//   string ToXX();
//   void ToXX(ostream&);
// DeSerialization:
//   bool FromXX(istream& instream);
//   bool FromXX(const string& s__)
//   bool FromXX(const char* s__, size_t len__)
// The return value specifies whether the deserialization succeeded or failed.
// If you need to serialize/deserialize something, it is recommended that you
// call the corresponding functions in Serializer namespace (defined below),
// which is more flexible and supports primitive data types as well as complex
// structs.
#define SERIALIZE(varlist___) \
  SERIALIZATION_DATA(varlist___);  \
  \
  SERIALIZATION_BINARY_IMPL(varlist___, inline); \
  \
  SERIALIZATION_RAW_BINARY_IMPL(varlist___, inline); \
  \
  SERIALIZATION_BINARY_INTERFACE(varlist___); \
  \
  SERIALIZATION_JSON_IMPL(varlist___, inline); \
  \
  SERIALIZATION_JSON_INTERFACE(varlist___) \

// SIGNATURE has the same syntax as SERIALIZE, but only supports one-way
// serialization (no deserialization).
#define SIGNATURE(varlist___) \
  SERIALIZATION_DATA(varlist___);  \
  \
  BINARY_SERIALIZATION_IMPL(varlist___, inline); \
  \
  RAW_BINARY_SERIALIZATION_IMPL(varlist___, inline); \
  \
  BINARY_SERIALIZATION_INTERFACE(varlist___); \
  \
  JSON_SERIALIZATION_IMPL(varlist___, inline); \
  \
  JSON_SERIALIZATION_INTERFACE(varlist___) \

#define RAW_BINARY_SIGNATURE(varlist___) \
  [&]() -> string { \
    stringstream ss__; \
    ::serial::SerializeRawBinary r__(ss__); \
    r__ / varlist___; \
    return ss__.str(); \
  }()


// ------------------------------------------------------------
// Virtual verions of public macros
// ------------------------------------------------------------
#define SERIALIZE_BINARY_VIRTUAL(varlist___)                                   \
  SERIALIZATION_DATA(varlist___);                                              \
                                                                               \
  SERIALIZATION_BINARY_IMPL(varlist___, inline virtual);                       \
                                                                               \
  SERIALIZATION_RAW_BINARY_IMPL(varlist___, inline virtual);                   \
                                                                               \
  SERIALIZATION_BINARY_INTERFACE(varlist___)                                   \

#define SERIALIZE_RAW_BINARY_VIRTUAL(varlist___)                               \
  SERIALIZATION_BINARY_DUMMY_IMPL(varlist___, inline virtual);                 \
                                                                               \
  SERIALIZATION_RAW_BINARY_IMPL(varlist___, inline virtual);                   \
                                                                               \
  SERIALIZATION_BINARY_INTERFACE(varlist___)                                   \

#define SERIALIZE_JSON_VIRTUAL(varlist___)                                     \
  SERIALIZATION_DATA(varlist___);                                              \
                                                                               \
  SERIALIZATION_JSON_IMPL(varlist___, inline virtual);                         \
                                                                               \
  SERIALIZATION_JSON_INTERFACE(varlist___)                                     \

#define SERIALIZE_VIRTUAL(varlist___)                                          \
  SERIALIZATION_DATA(varlist___);                                              \
                                                                               \
  SERIALIZATION_BINARY_IMPL(varlist___, inline virtual);                       \
                                                                               \
  SERIALIZATION_RAW_BINARY_IMPL(varlist___, inline virtual);                   \
                                                                               \
  SERIALIZATION_BINARY_INTERFACE(varlist___);                                  \
                                                                               \
  SERIALIZATION_JSON_IMPL(varlist___, inline virtual);                         \
                                                                               \
  SERIALIZATION_JSON_INTERFACE(varlist___)                                     \

#define SIGNATURE_VIRTUAL(varlist___)                                          \
  SERIALIZATION_DATA(varlist___);                                              \
                                                                               \
  BINARY_SERIALIZATION_IMPL(varlist___, inline virtual);                       \
                                                                               \
  RAW_BINARY_SERIALIZATION_IMPL(varlist___, inline virtual);                   \
                                                                               \
  BINARY_SERIALIZATION_INTERFACE(varlist___);                                  \
                                                                               \
  JSON_SERIALIZATION_IMPL(varlist___, inline virtual);                         \
                                                                               \
  JSON_SERIALIZATION_INTERFACE(varlist___)                                     \

// Macro for CSV.
// TODO(pramodg): Fix this.
#define CSV(varlist___) \
  bool FromCSV(const char *s___, char delimitor___, bool is_header_line___, \
               string *error_msg___ = NULL) { \
    DeserializeCSV r___(s___, delimitor___); \
    if (is_header_line___) \
      r___.CheckHeaderLine(#varlist___); \
    else { \
      r___ | varlist___; \
      r___.End(); \
    } \
    if (error_msg___ != NULL) \
      *error_msg___ = r___.get_error_msg(); \
    return r___.ok(); \
  } \
  inline bool FromCSV(const string& s___, char delimitor___, \
                      bool is_header_line___, string *error_msg___ = NULL) { \
    return FromCSV(s___.c_str(), delimitor___, \
                   is_header_line___, error_msg___);    \
  } \
  string ToCSV(char delimitor___ = ',', bool header_line___ = false) const { \
    if (header_line___) \
      return SerializeCSV::MakeCSVHeader(#varlist___, delimitor___); \
    else { \
      SerializeCSV s___(delimitor___); \
      s___ | varlist___; \
      return s___.str(); \
    } \
  }

#endif  // _PUBLIC_UTIL_SERIAL_SERIALIZER_MACROS_H_
