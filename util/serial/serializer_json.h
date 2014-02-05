// Copyright 2013 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

//--------------------------------------------------------------------------
// JSON Serialization.
// See http://json.org/ for quick overview of JSON format.
//--------------------------------------------------------------------------

#ifndef _PUBLIC_UTIL_SERIAL_SERIALIZER_JSON_H_
#define _PUBLIC_UTIL_SERIAL_SERIALIZER_JSON_H_

#include <istream>
#include <ostream>
#include <sstream>

#include "base/defs.h"
#include "util/serial/serialization_data.h"
#include "util/serial/type_handlers/deserialization_callback.h"
#include "util/serial/type_handlers/json_deserialization.h"
#include "util/serial/type_handlers/json_serialization.h"
#include "util/serial/utils/serializer_util.h"
#include "util/serial/utils/stream_util.h"

namespace serial {

class SerializeJSON {
  typedef SerializationData::FieldData SDField;

 public:
  explicit SerializeJSON(ostream& out, const SerializationData& data,
                         const JSONSerializationParams& params = JSONSerializationParams())
      : out_(out), data_(data), params_(params), serializer_(params) {}

 public:
  SerializeJSON& BeginIteration() {
    next_field_ = 0;
    need_separator_ = false;
    AddSeparator('{');
    params_.Indent();
    // Set the correct indentation for the serializer.
    serializer_.params.indent = params_.indent;

    // Check if the type has a pretty name that needs to be serialized.
    if (data_.pretty_type_name().size()) {
      AddLine();
      static const string kTypeField = "__type_info";
      AddString(kTypeField);

      // Add the separator.
      AddSeparator(':', true);
      AddString(data_.pretty_type_name());
      need_separator_ = true;
    }
    return *this;
  }

  SerializeJSON& EndIteration() {
    params_.Dedent();
    AddLine();
    AddSeparator('}');
    return *this;
  }

  template<typename Field>
  SerializeJSON& operator /(const Field& v) {
    // Add a separator, if required.
    if (next_field_ > 0 || need_separator_) AddSeparator(',');
    // Add line if required.
    AddLine();

    ASSERT_LT(next_field_, data_.name_id_list().size());
    const size_t field_id = data_.name_id_list()[next_field_++].second;
    const SDField* field_data = data_.field_data(field_id);

    ASSERT_NOTNULL(field_data);

    // Dump the field name.
    AddString(field_data->name);

    // Add the separator.
    AddSeparator(':', true);

    // Dump the value.
    serializer_(out(), v);
    return *this;
  }

  // Ignore the special flags.
  SerializeJSON& operator /(const SerializationHelper& v) {
    return *this;
  }

  // Ignore the * operators.
  template<typename Field>
  SerializeJSON& operator *(const Field& v) { return *this; }

 protected:
  ostream& out() { return out_;}

  // ------------------------------------------------------------
  // Utility functions.
  // ------------------------------------------------------------

  // Writes an escaped string to the stream.
  void AddString(const string& str) {
    out().put('"');
    out().write(str.c_str(), str.size());
    out().put('"');
  }

  // Add indentation, if requested.
  void AddLine() {
    if (params_.indent_increment > 0) {
      out().put('\n');
      string s(params_.indent, ' ');
      out().write(s.c_str(), s.size());
    }
  }

  void AddSpace() {
    if (params_.indent_increment > 0) out().put(' ');
  }

  void AddSeparator(const char sep, bool space = false) {
    out().put(sep); if (space) AddSpace();
  }

  ostream& out_;
  const SerializationData& data_;
  JSONSerializationParams params_;
  JSONSerializationByType serializer_;
  // The next field index in the iteration. This will match the indices in the
  // vector names_ exactly.
  size_t next_field_ = 0;
  bool need_separator_ = false;
};

class DeSerializeJSON {
  typedef SerializationData::FieldData SDField;

 public:
  explicit DeSerializeJSON(const SerializationData& data,
      const JSONDeSerializationParams& params = JSONDeSerializationParams())
      : data_(data), params_(params), deserializer_(params) {}

 public:
  bool Deserialize(istream& in, char* res, const string& type_name);

 protected:
  bool SkipComma(istream& in);

  const SerializationData& data_;
  JSONDeSerializationParams params_;
  JSONDeSerializationByType deserializer_;
};

}  // namespace serial

// Implementation for JSON serialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define JSON_SERIALIZATION_IMPL(varlist___, ...)                               \
  __VA_ARGS__ void ToJSONImpl(ostream& out__,                                  \
      const ::serial::JSONSerializationParams& params__) const {               \
    typename ::serial::SerializeJSON r__(out__,                                \
        this->GetSerializationData(), params__);                               \
    r__.BeginIteration() / varlist___;                                         \
    r__.EndIteration();                                                        \
  }                                                                            \

// Implementation for JSON deserialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define JSON_DESERIALIZATION_IMPL(varlist___, ...)                             \
  __VA_ARGS__ bool FromJSONImpl(istream& in__,                                 \
      const ::serial::JSONDeSerializationParams& params__) {                   \
    typedef typename ::util::tl::remove_rcv<decltype(*this)>::type __MyType;   \
    typename ::serial::DeSerializeJSON r__(                                    \
        this->GetSerializationData(), params__);                               \
    bool res__ = r__.Deserialize(in__, reinterpret_cast<char*>(this),          \
                                 typeid(__MyType).name());                     \
    if (res__) res__ = ::serial::DeserializationCallbackRunner()(this);        \
    return res__;                                                              \
  }                                                                            \

// SERIALIZE_JSON defines the functions for JSON serialization/deserialization.
// This assumes that SERIALIZATION_DATA would be defined in the struct as well.
#define SERIALIZATION_JSON_IMPL(varlist___, ...)                               \
  JSON_SERIALIZATION_IMPL(varlist___, __VA_ARGS__);                            \
                                                                               \
  JSON_DESERIALIZATION_IMPL(varlist___, __VA_ARGS__)                           \


#endif  // _PUBLIC_UTIL_SERIAL_SERIALIZER_JSON_H_
