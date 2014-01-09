// Copyright 2013 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/serial/serializer_json.h"

#include "util/serial/utils/json_util.h"

namespace serial {

bool DeSerializeJSON::Deserialize(istream& in, char* res, const string& type_name) {
  ASSERT(!in.fail());

  // Check if we are at end of file.
  in.peek();
  if (!in.good()) return false;

  istream::pos_type orig_pos = in.tellg();
  ASSERT_NE(static_cast<long>(orig_pos), -1);

  if (!util::ExpectNext(in, "{", true, params_.err)) return false;

  bool parsed_ids[data_.max_id() + 1];
  memset(parsed_ids, 0, data_.max_id() + 1);
  while (in.good() && util::SkipToNextChar(in) != '}' && in.good()) {
    istream::pos_type pos = in.tellg();

    static const string delims = " ,]}\n:";
    string key;
    util::ExtractQuotedString(in, &key, delims);
    if (in.fail()) break;

    // Check if there is a stray '@' in front of the name.
    if (key.size() && key[0] == '@') key = key.substr(1);

    // Find ':'
    if (in.fail() || !util::ExpectNext(in, ":", true, params_.err)) break;

    const SDField* field_data = data_.field_data(key);
    // Skip the field if it is unknown.
    if (field_data == nullptr) {
      VLOG(5) << "Could no get field data for field : " << key;
      util::JSONSkipAheadField(in);
      SkipComma(in);
      continue;
    }

    // The stream is good, parse the field.
    if (!in.fail()) {
      // Check if this is a null field.
      if (util::SkipToNextChar(in, false) == 'n') {
        util::SkipDelimitedString(in, delims);
        SkipComma(in);
        continue;
      }

      pos = in.tellg();
      char* field_offset = res + field_data->offset;
      field_data->json_deserializer(in, field_offset, deserializer_);
    }

    if (in.fail()) {
      util::LogParsingError(in, pos,
          string("Failed to parse field: ") + field_data->name, params_.err);
      break;
    }

    // Mark this id as parsed.
    parsed_ids[field_data->id] = true;

    // Next char can either be ',' or '}'.
    if (!SkipComma(in)) break;
  }

  if (in.fail()) {
    util::LogParsingError(in, orig_pos,
        string("Could not parse struct: ") + type_name,
        params_.err);
    return false;
  }

  if (!util::ExpectNext(in, "}", true, params_.err)) return false;

  // Handle Ids that were not deserialized.
  for (const auto& p : data_.id_map()) {
    if (parsed_ids[p.first]) continue;

    // This field was not deserialized.
    const SDField* field_data = p.second.get();
    ASSERT_NOTNULL(field_data);

    // Check if this field was required.
    if (field_data->required) {
      // Required fields must be part of the JSON.
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

bool DeSerializeJSON::SkipComma(istream& in) {
  // Next char can either be ',' or '}'.
  char ch = util::ExpectNext(in, ",}", false, params_.err);
  if (!ch) return false;
  if (ch == ',') util::SkipToNextChar(in, true);
  return true;
}

}  // namespace serial
