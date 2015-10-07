// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Interface for converting abbreviations to completions and vice-versa.

#ifndef _UTIL_ABBREVIATIONS_ABBREVIATIONS_H_
#define _UTIL_ABBREVIATIONS_ABBREVIATIONS_H_

#include "base/common.h"
#include "util/factory/factory.h"
#include "util/factory/factory_extra.h"
#include "util/string/strutil.h"

namespace util {
namespace abbr {

class Abbreviation : public Factory<Abbreviation> {
 public:
  virtual ~Abbreviation() {}
  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }
  // Initialize the class.
  virtual bool Initialize() { return true; }

  // Returns true if the input string is an abbreviation.
  virtual bool IsAbbreviation(const string& abbr) const = 0;

  // Returns the completion of the string if one exists. Else returns the same
  // string back.
  virtual string GetCompletion(const string& abbr) const = 0;

  // Returns true if the input string is a complete word for which an
  // abbreviation exists.
  virtual bool IsCompletion(const string& complete) const = 0;

  // Returns the abbreviation of the string if one exists. Else returns the same
  // string back.
  virtual string GetAbbreviation(const string& complete) const = 0;

  // Replaces all the abbreviations in the phrase with the corresponding
  // completions.
  virtual string ReplaceAllAbbreviations(const string& phrase,
                                         const string& delim = " ") const {
    vector<string> words;
    string res;
    strutil::SplitString(phrase, delim, &words);
    for (const string& word : words) {
      if (!res.empty()) res += delim;
      res += GetCompletion(word);
    }
    return res;
  }

  // Replaces all the completions in the phrase with the corresponding
  // abbreviations.
  virtual string ReplaceAllCompletions(const string& phrase,
                                       const string& delim = " ") const {
    vector<string> words;
    string res;
    strutil::SplitString(phrase, delim, &words);
    for (const string& word : words) {
      if (!res.empty()) res += delim;
      res += GetAbbreviation(word);
    }
    return res;
  }
};

vector<string> GetAllVariants(const string& phrase, const string& delim = " ");

}  // namespace abbr
}  // namespace util


#endif  // UTIL_ABBREVIATIONS_ABBREVIATIONS_H_
