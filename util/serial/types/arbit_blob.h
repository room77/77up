// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// This class is a Birthday Gift to one Nicholas Edelman.

#ifndef _PUBLIC_UTIL_SERIAL_TYPES_ARBIT_BLOB_H_
#define _PUBLIC_UTIL_SERIAL_TYPES_ARBIT_BLOB_H_

#include <istream>
#include <ostream>
#include <sstream>

#include "base/defs.h"
#include "util/serial/serializer.h"
#include "util/serial/utils/json_util.h"

namespace serial {
namespace types {

// A struct representing an arbitrary JSON blob in an input JSON stream.
// WARNING: This struct should be used only in very rare circumstances (possibly due to an extreme
// C++ limitation). More often than not there are better solutions than using this class.
// Please discuss with eng@ before deciding to use this struct for any purpose.
struct ArbitBlob {
  ArbitBlob() = default;
  ArbitBlob(const ArbitBlob& other) = default;
  ArbitBlob(const string& s) : str(s) {}  // NOLINT
  ArbitBlob(const char* s) : str(s) {}  // NOLINT

  // Serialization Methods.

  // From Binary and ToBinary remain exactly the same.
  void ToBinary(std::ostream& out) const {
    // TODO(pramodg): Implement this.
    ASSERT(false);
  }

  bool FromBinary(std::istream& in) {
    // TODO(pramodg): Implement this.
    ASSERT(false);
    return false;
  }

  // This is where this struct helps. It allows us to directly dump wrapped structs without having
  // to escape them as a string again.
  void ToJSON(std::ostream& out) const {
    out << (str.empty() ? "null" : str);
  }

  // When we deserialize the field, we absorb the entire JSON blob representing the field as a
  // a string and make sure that deserialization completely skips this field.
  bool FromJSON(std::istream& in) {
    bool res = util::JSONSkipAheadAndReturnField(in, &str);
    // The stream is null set it to empty.
    if (str.size() == 4 && str == "null") str.clear();
    return res;
  }

  // Utility Functions.
  // From Binary and ToBinary remain exactly the same.
  string ToBinary() const {
    std::ostringstream ss;
    ToBinary(ss);
    return ss.str();
  }

  bool FromBinary(const string& str) {
    istringstream ss(str);
    return FromBinary(ss);
  }

  string ToJSON() const {
    std::ostringstream ss;
    ToJSON(ss);
    return ss.str();
  }

  bool FromJSON(const string& str) {
    istringstream ss(str);
    return FromJSON(ss);
  }

  bool empty() const {
    return str.empty();
  }

  void clear() { str.clear(); }

  // Utility operator to cast the blob as string.
  operator string() const {
    return str;
  }

  friend ostream& operator<<(ostream& stream, const ArbitBlob& b) {
    stream << b.str;
    return stream;
  }

  // The string corresponding to the JSON blob abosrbed from a JSON stream.
  string str;
};

} // namespace types
}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_TYPES_ARBIT_BLOB_H_
