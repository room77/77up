// Copyright 2013 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/serializer_binary.h"

namespace serial {

bool DeSerializeBinary::Deserialize(istream& in, char* res, const string& type_name) {
  ASSERT(!in.fail());

  // Check if we are at end of file.
  in.peek();
  if (!in.good()) return false;

  istream::pos_type orig_pos = in.tellg();
  ASSERT_NEQ(static_cast<long>(orig_pos), -1);

  bool parsed_ids[data_.max_id() + 1];
  memset(parsed_ids, 0, data_.max_id() + 1);
  while (in.good()) {
    // Get the next id.
    size_t serial_id = 0;
    deserializer_(in, &serial_id);
    if (in.fail()) break;

    // Check if we are done.
    if (serial_id == 0) break;

    pair<size_t, int> id_size = SerializedIdToIdAndSize(serial_id);
    if (id_size.first == 0) {
      util::LogParsingError(in, -1, "Invalid id 0", params_.err);
      break;
    }

    // Get the field data for the id.
    const SDField* field_data = data_.field_data(id_size.first);
    if (field_data == nullptr) {
      VLOG(5) << "Could no get field data for id : " << id_size.first;
      SkipAheadUnknownId(in, id_size.second);
      continue;
    }

    // Skip ahead by relevant size.
    SkipAheadKnownId(in, id_size.second);

    // The stream is good, parse the field.
    if (!in.fail()) {
      char* field_offset = res + field_data->offset;
      field_data->binary_deserializer(in, field_offset, deserializer_);
    }
    if (in.fail()) {
      util::LogParsingError(in, -1,
          string("Failed to parse field: ") + field_data->name, params_.err);
      break;
    }

    // Mark this id as parsed.
    parsed_ids[field_data->id] = true;
  }

  if (in.fail()) {
    util::LogParsingError(in, orig_pos,
        string("Could not parse struct: ") + type_name, params_.err);
    return false;
  }

  // Handle Ids that were not deserialized.
  for (const auto& p : data_.id_map()) {
    if (parsed_ids[p.first]) continue;

    // This field was not deserialized.
    const SDField* field_data = p.second.get();
    ASSERT_NOTNULL(field_data);

    // Check if this field was required.
    if (field_data->required) {
      // Required fields must be part of the serialized data.
      util::LogParsingError(in, orig_pos,
          string("Required field not found: ") + field_data->name, params_.err);
      return false;
    }

    // Set Zero default if required.
    if (field_data->zero_default) {
      char* field_offset = reinterpret_cast<char*>(res) + field_data->offset;
      field_data->zero_defaulter(field_offset);
    }
  }
  return true;
}

void DeSerializeBinary::SkipAheadUnknownId(istream& in, int type) {
  switch (type) {
    case kSerialTypeSizeVarInt : {
      size_t size = 0;
      deserializer_(in, &size);
      break;
    }
    case kSerialTypeUnknownVarInt : {
      size_t size = 0;
      deserializer_(in, &size);
      if (in.fail()) {
        util::LogParsingError(in, -1, "Failed to skip ahead.", params_.err);
        break;
      }
      in.seekg(in.tellg() + static_cast<long>(size));
      break;
    }
    case kSerialTypeUnknownFixedInt : {
      fixedint<unsigned int> size;
      deserializer_(in, &size);
      if (in.fail()) {
        util::LogParsingError(in, -1, "Failed to skip ahead.", params_.err);
        break;
      }
      in.seekg(in.tellg() + static_cast<long>(size));
      break;
    }
    case kSerialTypeSizeFour : in.seekg(in.tellg() + 4l); break;
    case kSerialTypeSizeEight : in.seekg(in.tellg() + 8l); break;
    default: {
      util::LogParsingError(in, in.tellg(), "Invalid Type", params_.err);
      break;
    }
  }
}

void DeSerializeBinary::SkipAheadKnownId(istream& in, int type) {
  switch (type) {
    case kSerialTypeUnknownFixedInt : {
      fixedint<unsigned int> size  = 0;
      deserializer_(in, &size);
      break;
    }
    default: break;
  }
}

}  // namespace serial
