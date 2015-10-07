// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "util/region_data/utils.h"

// #include <mutex>  // Read comment in NormalizeString

#include "util/string/strutil.h"
#include "util/abbreviations/abbreviations.h"
#include "util/string/unicode.h"
#include "util/region_data/synonyms.h"
#include "util/templates/container_util.h"

FLAG_int(region_min_critical_word_length, 4,
         "Minimum length a word must have for it to be considered critical.");

namespace region_data {
namespace utils {

string GetCommaUnMangledName(const string& name) {
  vector<string> parts;
  if (strutil::SplitStringNParts(name, ",", &parts, 1) != 2) return "";
  return strutil::GetTrimmedString(parts[1]) + " " + strutil::GetTrimmedString(parts[0]);
}

string NormalizeString(const string& str) {
  static unicode::NormalizeForIndexing normalizer;
  string res = normalizer(str);
  res = strutil::NormalizeName(res.empty() ? str : res);
  return res;
}

bool GetAlternateNames(const string& name, unordered_set<string>* res) {
  // Get the unmangled alternate name.
  string unmangled_name = GetCommaUnMangledName(name);
  if (!unmangled_name.empty())
    res->insert(NormalizeString(unmangled_name));

  // Get the name without brackets.
  size_t begin = name.find_first_of("[(");
  if (begin != string::npos) {
    string without_brackets = name.substr(0, begin);
    size_t end = name.find_first_of("])", begin);

    if (end != string::npos && end + 1 < name.size())
      without_brackets += name.substr(end + 1);

    res->insert(NormalizeString(without_brackets));
  }
  return res->size();
}

}  // namespace utils
}  // namespace region_data
