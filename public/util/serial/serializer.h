// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_UTIL_SERIAL_SERIALIZER_H_
#define _PUBLIC_UTIL_SERIAL_SERIALIZER_H_

#include "base/logging.h"

#include "util/serial/serializer_macros.h"
#include "util/serial/type_handlers/binary_deserialization.h"
#include "util/serial/type_handlers/binary_serialization.h"
#include "util/serial/type_handlers/json_deserialization.h"
#include "util/serial/type_handlers/json_serialization.h"

namespace serial {

class Serializer {
 public:
  //-------------------------------------------------
  // Binary serialization interface.
  //-------------------------------------------------
  template<typename T>
  static void ToBinary(ostream& out, const T& v,
      const BinarySerializationParams& params = BinarySerializationParams()) {
    BinarySerializationByType serializer(params);
    serializer(out, v);
  }

  template<typename T>
  static string ToBinary(const T& v,
      const BinarySerializationParams& params = BinarySerializationParams()) {
    ostringstream ss;
    ToBinary(ss, v, params);
    return ss.str();
  }

  // Utility function to prepend size in front of the struct serialized.
  template<typename SizeT = unsigned int, typename T>
  static void ToBinaryPrependSize(ostream& out, const T& v,
      const BinarySerializationParams& params = BinarySerializationParams()) {
    ostream::pos_type pos = out.tellp();
    fixedint<SizeT> size = 0;
    ToBinary(out, size, params);
    ToBinary(out, v, params);
    ostream::pos_type new_pos = out.tellp();
    size = new_pos - pos - sizeof(unsigned int);
    out.seekp(pos);
    ToBinary(out, size, params);
    out.seekp(new_pos);
  }

  template<typename SizeT = unsigned int, typename T>
  static string ToBinaryPrependSize(const T& v,
      const BinarySerializationParams& params = BinarySerializationParams()) {
    ostringstream ss;
    ToBinaryPrependSize<SizeT>(ss, v, params);
    return ss.str();
  }

  //-------------------------------------------------
  // Binary deserialization interface.
  //-------------------------------------------------
  template<typename T>
  static bool FromBinary(istream& in, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    BinaryDeSerializationByType deserializer(params);
    deserializer(in, v);
    return !in.fail();
  }

  template<typename T>
  static bool FromBinary(const string& s, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    istringstream ss(s);
    return FromBinary(ss, v, params);
  }

  template<typename T>
  static bool FromBinary(const char* s, size_t len, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    return FromBinary(string(s, len), v, params);
  }

  // Utility function to deserialize a struct where size had been prepended.
  template<typename SizeT = unsigned int, typename T>
  static bool FromBinaryPrependedSize(istream& in, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    fixedint<SizeT> size = 0;
    if (!FromBinary(in, &size, params)) return false;
    return FromBinary(in, v, params);
  }

  template<typename SizeT = unsigned int, typename T>
  static bool FromBinaryPrependedSize(const string& s, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    istringstream ss(s);
    return FromBinaryPrependedSize<SizeT>(ss, v, params);
  }

  template<typename SizeT = unsigned int, typename T>
  static bool FromBinaryPrependedSize(const char* s, size_t len, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    return FromBinaryPrependedSize<SizeT>(string(s, len), v, params);
  }

  // Returns true if the binary stream has enough data for FromBinaryPrependedSize to succeed.
  // TODO(pramodg): Write similar functions for streams.
  template<typename SizeT = unsigned int>
  static bool BinaryStreamHasEnoughDataToParsePrependedSize(const string& str) {
    if (str.size() < sizeof(SizeT)) return false;

    // Get the size in the prefix.
    fixedint<SizeT> size = 0;
    if (!FromBinary(str.substr(0, sizeof(SizeT)), &size)) return false;

    // Check if the stream size is large enough.
    if (str.size() < size + sizeof(SizeT)) {
      VLOG(5) << "Stream size too short. Expected: "
              << size + sizeof(unsigned int) << ", Actual: " << str.size();
      return false;
    }
    return true;
  }

  //-------------------------------------------------
  // Raw Binary serialization interface.
  //-------------------------------------------------
  template<typename T>
  static void ToRawBinary(ostream& out, const T& v,
      const BinarySerializationParams& params = BinarySerializationParams()) {
    BinarySerializationParams local_params = params;
    local_params.raw_binary = true;
    return ToBinary(out, v, local_params);
  }

  template<typename T>
  static string ToRawBinary(const T& v,
      const BinarySerializationParams& params = BinarySerializationParams()) {
    BinarySerializationParams local_params = params;
    local_params.raw_binary = true;
    return ToBinary(v, local_params);
  }

  //-------------------------------------------------
  // Raw Binary deserialization interface.
  //-------------------------------------------------
  template<typename T>
  static bool FromRawBinary(istream& in, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    BinaryDeSerializationParams local_params = params;
    local_params.raw_binary = true;
    return FromBinary(in, v, local_params);
  }

  template<typename T>
  static bool FromRawBinary(const string& s, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    BinaryDeSerializationParams local_params = params;
    local_params.raw_binary = true;
    return FromBinary(s, v, local_params);
  }

  template<typename T>
  static bool FromRawBinary(const char* s, size_t len, T* v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    BinaryDeSerializationParams local_params = params;
    local_params.raw_binary = true;
    return FromBinary(s, len, v, local_params);
  }

  //-------------------------------------------------
  // JSON serialization interface.
  //-------------------------------------------------
  template<typename T>
  static void ToJSON(ostream& out, const T& v,
      const JSONSerializationParams& params = JSONSerializationParams()) {
    JSONSerializationByType serializer(params);
    serializer(out, v);
  }

  template<typename T>
  static string ToJSON(const T& v,
      const JSONSerializationParams& params = JSONSerializationParams()) {
    ostringstream ss;
    ToJSON(ss, v, params);
    return ss.str();
  }

  //-------------------------------------------------
  // JSON deserialization interface.
  //-------------------------------------------------
  template<typename T>
  static bool FromJSON(istream& in, T* v,
      const JSONDeSerializationParams& params = JSONDeSerializationParams()) {
    JSONDeSerializationByType deserializer(params);
    deserializer(in, v);
    return !in.fail();
  }

  template<typename T>
  static bool FromJSON(const string& s, T* v,
      const JSONDeSerializationParams& params = JSONDeSerializationParams()) {
    istringstream ss(s);
    return FromJSON(ss, v, params);
  }

  template<typename T>
  static bool FromJSON(const char* s, size_t len, T* v,
      const JSONDeSerializationParams& params = JSONDeSerializationParams()) {
    return FromJSON(string(s, len), v, params);
  }

  // TODO(pramodg): The ToBase64 and FromBase64 is a temporary hack for
  // serialization change so that clients can still make bookings during the
  // change.

  // TODO(pramodg): Move these functions to encoding.
  // serialize a single variable to RPC format, then encode in Base64
  template<typename T>
  static inline string ToBase64(const T& input,
      const BinarySerializationParams& params = BinarySerializationParams()) {
    return strutil::EncodeString_Base64(ToBinary(input, params));
  }

  // decode Base64 string, then deserialize from RPC format
  template<typename T>
  static inline bool FromBase64(const string& encoded, T *v,
      const BinaryDeSerializationParams& params = BinaryDeSerializationParams()) {
    const string str = strutil::DecodeString_Base64(encoded);
    bool status = FromBinary(str, v, params);
    if (!status) status = FromJSON(str, v);
    return status;
  }
};

}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_SERIALIZER_H_
