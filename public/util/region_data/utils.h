// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Common utils for region data.

#ifndef _UTIL_REGION_DATA_UTILS_H_
#define _UTIL_REGION_DATA_UTILS_H_

#include <cmath>
#include <unordered_set>
#include <utility>

#include "base/common.h"
#include "util/string/strutil.h"

extern int gFlag_region_min_critical_word_length;

namespace region_data {
namespace utils {

// Returns the name of the country after removing the ',' and formatting it
// correctly. e.g. the name 'Virgin Islands, U.S.' is formatted to
// 'U.S. Virgin Islands'. If unmangling is not possible, empty string is
// returned.
string GetCommaUnMangledName(const string& name);

// Returns a normalized version of the string after removing extra spaces,
// accents, etc.
string NormalizeString(const string& str);

// Generates alternate names for the name after comma unmangling and bracket
// removal. Fills in the set. Returns true if alternate names were generated
// and false otherwise.
bool GetAlternateNames(const string& name, unordered_set<string>* res);

// Returns concatenated alternate names for serialization.
inline string GetAlternateNames(const string& name) {
  unordered_set<string> names;
  if (GetAlternateNames(name, &names)) {
    return strutil::JoinString(names, "|");
  }
  return "";
}

// Returns true if the lat long is valid and false otherwise.
inline bool IsValidLatLong(double lat, double lng) {
  // The condition "lat != lng" is a hack because EAN has incorrect entries
  // where longitude / latitude values are the same.
  return ((lat != 0 || lng != 0) && fabs(lat) <= 90 && fabs(lng) <= 180 &&
          (lat != lng));
}

// Returns the mismatch extent between a string and a query.
// Note that the score is not symmetric. 'str' is usually the superset string of
// 'query'. The higher the returned value the more is the mismatch.
// In case the two strings have nothing in common '-1' is returned.
// Note: we assume that str and query are already normalized using
// 'NormalizeString' function (or similar) if necessary.
inline double GetMismatchExtent(const string& str, const string& query,
    int min_critical_word_length = gFlag_region_min_critical_word_length) {
  // If the complete query is already in the str, we simply return the
  // score based on the amount of offset. This ensures, strings with query
  // appearing earlier will have lower scores.
  size_t pos = str.find(query);
  if (pos != string::npos) return pos * query.size();

  // Split words and find word mismatch.
  vector<string> words;
  strutil::SplitString(query, " ", &words);

  double score = 0;
  int query_pos = 0;
  for (const string& word : words) {
    size_t str_pos = str.find(word);
    if (str_pos != string::npos) {
      score += abs(str_pos - query_pos) * word.size();
    } else {
      if (word.size() < min_critical_word_length)
        score += str.size() * word.size();
      else {
        score = -1;
        break;
      }
    }
    query_pos += word.size() + 1;  // 1 is for the space.
  }
  return score;
}

// Returns the least mismatch extent between a set of strings and a query.
// Each string in 'names' is usually the superset string of 'query'.
// The higher the returned value the more is the mismatch.
// In case none of the strings have anything in common with the query '-1' is
// returned. Note: we assume that query is already normalized using
// 'NormalizeString' function (or similar) if necessary.
template <typename C>
double GetLeastMismatchExtent(const C& names, const string& query,
    int min_critical_word_length = gFlag_region_min_critical_word_length) {
  double min_score = std::numeric_limits<double>::max();
  for (const string& name : names) {
    const string norm_name = utils::NormalizeString(name);
    const double score = GetMismatchExtent(norm_name, query,
                                           min_critical_word_length);
    if (score == -1) continue;
    min_score = std::min<double>(min_score, score);
  }

  return min_score < std::numeric_limits<double>::max() ? min_score : -1;
}

// Pair of <city name, < state iso code, country iso code>>
typedef pair<string, pair<string, string>> tCityStateCountryKey;

// Generates the city state country key.
inline tCityStateCountryKey CityStateCountryKey(const string& city,
    const string& state_code, const string& country_code) {
  return make_pair(utils::NormalizeString(city),
                   make_pair(utils::NormalizeString(state_code),
                             utils::NormalizeString(country_code)));
}

// Generates the city country key.
inline pair<string, string> CityCountryKey(const string& city,
                                           const string& country_code) {
  return make_pair(utils::NormalizeString(city),
                   utils::NormalizeString(country_code));
}

}  // namespace utils
}  // namespace region_data

#endif  // UTIL_REGION_DATA_UTILS_H_
