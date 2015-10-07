// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <unordered_map>

#include "util/abbreviations/abbreviations.h"

#include "base/defs.h"
#include "util/file/csvreader.h"
#include "util/hash/hash_util.h"
#include "util/serial/serializer.h"

FLAG_string(street_abbreviation_params,
    "{"
       "\"file\": \"static_data/push/auto/region_data/abbreviations/street.csv\","
    "}",
    "Default parameters for street abbreviation map.");

FLAG_string(name_abbreviation_params,
    "{"
       "\"file\": \"static_data/push/auto/region_data/abbreviations/name.csv\","
    "}",
    "Default parameters for name abbreviation map.");


namespace util {
namespace abbr {

struct tAbbr {
  // The complete name.
  string name;
  // The Abbreviation.
  string abbr;

  CSV(name | abbr);
};

class AbbreviationNameMap : public Abbreviation {
  struct AbbreviationNameMapConfigParams {
    string file;
    SERIALIZE(file*1);
  };

 public:
  virtual ~AbbreviationNameMap() {}
  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) {
    return params_.FromJSON(opts);
  }

  // Initialize the class.
  virtual bool Initialize() {
    CSV::CSVReader<vector<tAbbr> > reader(params_.file);
    vector<tAbbr> file;
    if (!reader.Read(&file)) {
      LOG(INFO) << "Could not read file. Params: " << params_.ToJSON();
      return false;
    }
    for (const tAbbr& abbr : file) {
      abbr_name_map_[abbr.abbr] = abbr.name;
      name_abbr_map_[abbr.name] = abbr.abbr;
    }
    return abbr_name_map_.size();
  }

  // Returns true if the input string is an abbreviation.
  virtual bool IsAbbreviation(const string& abbr) const {
    const auto iter = abbr_name_map_.find(strutil::NormalizeName(abbr));
    return iter != abbr_name_map_.end() ? true : false;
  }

  // Returns the completion of the string if one exists. Else returns the same
  // string back.
  virtual string GetCompletion(const string& abbr) const {
    const auto iter = abbr_name_map_.find(strutil::NormalizeName(abbr));
    return iter != abbr_name_map_.end() ? iter->second : abbr;
  }

  // Returns true if the input string is a complete word for which an
  // abbreviation exists.
  virtual bool IsCompletion(const string& complete) const {
    const auto iter = name_abbr_map_.find(strutil::NormalizeName(complete));
    return iter != name_abbr_map_.end() ? true : false;
  }

  // Returns the abbreviation of the string if one exists. Else returns the same
  // string back.
  virtual string GetAbbreviation(const string& complete) const {
    const auto iter = name_abbr_map_.find(strutil::NormalizeName(complete));
    return iter != name_abbr_map_.end() ? iter->second : complete;
  }

 private:
  AbbreviationNameMapConfigParams params_;

  typedef unordered_map<string, string, ::hash::string_casefold_hash,
      ::hash::string_casefold_eq> CaseInsensitiveMap;
  CaseInsensitiveMap abbr_name_map_;
  CaseInsensitiveMap name_abbr_map_;
};

// Register different abbreviation types.
auto register_street_abbreviations = Abbreviation::bind("street_abbr",
    InitializeConfigureConstructor<AbbreviationNameMap, string>(
        gFlag_street_abbreviation_params));

auto register_name_abbreviations = Abbreviation::bind("name_abbr",
    InitializeConfigureConstructor<AbbreviationNameMap, string>(
        gFlag_name_abbreviation_params));

}  // namespace abbr
}  // namespace util
