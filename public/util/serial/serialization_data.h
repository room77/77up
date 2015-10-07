// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_UTIL_SERIAL_SERIALIZATION_DATA_H_
#define _PUBLIC_UTIL_SERIAL_SERIALIZATION_DATA_H_

#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>

#include "base/defs.h"

#include "util/serial/type_handlers/binary_deserialization.h"
#include "util/serial/type_handlers/json_deserialization.h"
#include "util/serial/type_handlers/zero_default.h"
#include "util/templates/hash.h"
#include "util/templates/type_traits.h"

namespace serial {

class SerializationData {
 public:
  // Per field serialization data.
  struct FieldData {
    // The numeric id for the field.
    string name;

    // The numeric id for the field.
    size_t id = 0;

    // The memory offset of the field.
    size_t offset = 0;

    // Hash of type of the field.
    // Note that this hash is not persistent across exe invocations.
    size_t type_hash = 0;

    // Set to true if the value should have a default value of zero.
    bool zero_default = true;

    // Set to true if the value should be always serialized regardless whether
    // it is the same as default or not.
    bool always_serialize = false;

    // Whether the field is required or not.
    bool required = false;

    // Methods for Binary deserialization.
    std::function<void (istream&, char*, BinaryDeSerializationByType&)>
        binary_deserializer;

    // Methods for JSON deserialization.
    std::function<void (istream&, char*, JSONDeSerializationByType&)>
        json_deserializer;

    // Method for Default setting.
    std::function<void (char*)> zero_defaulter;
  };

  SerializationData(const char* obj, const std::string& type_info_name)
      : obj_(obj), type_info_name_(type_info_name) {}
  ~SerializationData() {}

  // Map from name -> field data.
  typedef vector<pair<string, size_t>> NameIdList;
  typedef unordered_map<string, shared_ptr<FieldData>> FieldNameMap;
  typedef unordered_map<size_t, shared_ptr<FieldData>,
      ::util::tl::identity_hash<size_t>> FieldIdMap;

  void Initialize(const string& varlist);

  const FieldData* field_data(const string& field) const {
    const auto p = name_map().find(field);
    return p != name_map().end() ? p->second.get() : nullptr;
  }

  const FieldData* field_data(const size_t& id) const {
    const auto p = id_map().find(id);
    return p != id_map().end() ? p->second.get() : nullptr;
  }

  const char* obj() const { return obj_; }

  const string& type_info_name() const { return type_info_name_; }

  const string& pretty_type_name() const { return pretty_type_name_; }

  const NameIdList& name_id_list() const { return name_id_list_; }

  const FieldNameMap& name_map() const { return field_name_map_; }

  const FieldIdMap& id_map() const { return field_id_map_; }

  size_t max_id() const { return max_id_; }

  void DebugField(ostream& out, const string& name) const;

  string DebugField(const string& name) const;

  string DebugString() const;

 public:
  // Functions for evaluating different field objects.

  SerializationData& BeginIteration() {
    next_field_ = 0;
    return *this;
  }

  template<typename Field>
  SerializationData& operator /(Field& v) {  // NOLINT
    ASSERT_LT(next_field_, name_id_list_.size());
    shared_ptr<FieldData> field_data =
        field_id_map_[name_id_list_[next_field_++].second];
    ASSERT_NOTNULL(field_data);

    // Set the offset for this field.
    field_data->offset =
        static_cast<size_t>(reinterpret_cast<const char*>(&v) - obj_);

    // Set the type.
    field_data->type_hash = typeid(Field).hash_code();

    // Set Zero Default.
    if (field_data->zero_default) DefaultZeroByType()(v);

    // Add the deserializer.
    field_data->binary_deserializer =
        [](istream& in, char* ptr, BinaryDeSerializationByType& deserializer) {
            deserializer(in, reinterpret_cast<Field*>(ptr));
        };

    field_data->json_deserializer =
        [](istream& in, char* ptr, JSONDeSerializationByType& deserializer) {
            deserializer(in, reinterpret_cast<Field*>(ptr));
        };

    field_data->zero_defaulter =
        [](char* ptr) { DefaultZeroByType()(*(reinterpret_cast<Field*>(ptr)));};

    return *this;
  }

  // Ignore the special flags.
  SerializationData& operator /(SerializationHelper& v) {  // NOLINT
    return *this;
  }

  // Ignore the * operators.
  template<typename Field>
  SerializationData& operator *(const Field& v) { return *this; }

 protected:
  // A pointer to the default object for the type;
  const char* obj_ = nullptr;

  // The type info name of the object. e.g 3fooISsLi10EE
  string type_info_name_;

  // The pretty type info name of the object. e.g. foo<std::string, 10>
  string pretty_type_name_;

  // Names and ids in a specific order.
  // This order matches the order of calls to the '/' operator.
  NameIdList name_id_list_;

  // Field data keyed by name.
  FieldNameMap field_name_map_;

  // Field data keyed by id.
  FieldIdMap field_id_map_;

  // The next field index in the iteration. This will match the indices in the
  // vector names_ exactly.
  size_t next_field_ = 0;

  // The value of max id.
  size_t max_id_ = 0;
};

}  // namespace serial

// The init_serial_data__ and def_serial_data__ should only be called for
// default_obj and no other class object. All other objects should call
// GetSerializationData().
#define SERIALIZATION_DATA(varlist___) \
  auto default_obj__() const -> typename ::util::tl::remove_rcv<decltype(*this)>::type& { \
    static typename ::util::tl::remove_rcv<decltype(*this)>::type def_obj; \
    return def_obj; \
  } \
  \
  ::serial::SerializationData init_serial_data__() { \
    ::serial::SerializationData data__(reinterpret_cast<char*>(this), \
                                       typeid(decltype(*this)).name()); \
    data__.Initialize(#varlist___); \
    data__.BeginIteration() / varlist___; \
    return data__; \
  } \
  \
  ::serial::SerializationData& def_serial_data__() { \
    static ::serial::SerializationData data__ = init_serial_data__(); \
    return data__; \
  } \
  ::serial::SerializationData& GetSerializationData() const { \
    return default_obj__().def_serial_data__(); \
  } \
  \
  void DebugSerializationData() { \
    LOG(INFO) << GetSerializationData().DebugString(); \
  } \

#endif  // _PUBLIC_UTIL_SERIAL_SERIALIZATION_DATA_H_
