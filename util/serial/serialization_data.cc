// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/serialization_data.h"

#include <algorithm>
#include <memory>
#include <unordered_set>

#include "base/demangle.h"
#include "base/logging.h"
#include "util/string/strutil.h"
#include "util/templates/container_util.h"

namespace serial {

void SerializationData::Initialize(const string& varlist) {
  string norm_varlist = strutil::ReplaceAll(varlist, " ", "");
  vector<string> vars;
  strutil::SplitString(norm_varlist, "/", &vars);

  bool always_serialize = false;
  bool zero_default = true;
  bool required = false;
  unordered_set<size_t> used;
  name_id_list_.reserve(vars.size());
  for (const string& var : vars) {
    // Check for special flags.
    if (var == "DEFAULT_ZERO") {
      zero_default = true;
      continue;
    } else if (var == "DEFAULT_CUSTOM") {
      zero_default = false;
      continue;
    } else if (var == "SERIALIZE_REQUIRED") {
      required = true;
      continue;
    } else if (var == "SERIALIZE_OPTIONAL") {
      required = false;
      continue;
    } else if (var == "SERIALIZE_MODIFIED") {
      always_serialize = false;
      continue;
    } else if (var == "SERIALIZE_ALWAYS") {
      always_serialize = true;
      continue;
    } else if (var == "SERIALIZE_TYPEINFO") {
      pretty_type_name_ = base::PrettyNameFromTypeInfo(type_info_name_);
      continue;
    }

    // We have a valid var.
    vector<string> fields;
    if (!strutil::SplitString(var, "*%", &fields)) continue;
    ASSERT(fields.size() > 1) << "Must specify id for : " << var;

    string& name = fields[0];
    ASSERT(name.size()) << "Name must be non empty: " << var;

    // Remove trailing underscore.
    if (name.back() == '_') name.resize(name.size() - 1);

    ASSERT(field_name_map_.find(name) == field_name_map_.end())
        << " Field name " << name << " already in use. " << DebugField(name);

    size_t id = strutil::FromString<unsigned int>(fields[1]);

    ASSERT_NE(0, id) << "Id 0 is reserved. "
                      << "Please choose another id for field: " << name;

    ASSERT(used.find(id) == used.end()) << "Id " << id
        << " for field " << name << " already in use.";

    used.insert(id);

    // Create data for the new field.
    shared_ptr<FieldData> field_data(new FieldData);
    field_data->id = id;
    field_data->required = required;
    field_data->name = name;
    field_data->zero_default = zero_default;
    field_data->always_serialize = always_serialize;

    // Add the name and id to the ordered list.
    name_id_list_.push_back(make_pair(name, id));

    // Add the field data to different maps.
    field_name_map_[field_data->name] = field_data;
    field_id_map_[field_data->id] = field_data;

    max_id_ = std::max(id, max_id_);
  }
}

void SerializationData::DebugField(ostream& out, const string& name) const {
  const auto& p = ::util::tl::FindOrDie(name_map(), name);
  const shared_ptr<FieldData>& field_data = p.second;
  out << name << ": name=" << field_data->name
      << ", id=" << static_cast<int>(field_data->id)
      << ", offset=" << field_data->offset
      << ", type_hash=" << field_data->type_hash
      << ", zero=" << field_data->zero_default
      << ", force=" << field_data->always_serialize << endl;
}

string SerializationData::DebugField(const string& name) const {
  stringstream ss;
  DebugField(ss, name);
  return ss.str();
}

string SerializationData::DebugString() const {
  stringstream ss;
  ss << "Type: " << (pretty_type_name_.size() ? pretty_type_name_ : type_info_name_) << endl;
  for (const auto& p : name_id_list()) DebugField(ss, p.first);
  return ss.str();
}

}  // namespace serial
