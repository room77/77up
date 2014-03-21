// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/util/suggest_utils.h"

#include "util/string/strutil.h"
#include "util/entity/entity_type.h"

namespace suggest {
namespace util {

const string GetAlgoNameFromType(const SugggestionAlgoType& type) {
  if (type == kCompletionAlgoTypeInvalid) return "invalid algo";

  vector<string> names;
  if (type & kCompletionAlgoTypePrefix) names.push_back("prefix");
  if (type & kCompletionAlgoTypeMidString) names.push_back("mid_string");
  if (type & kCompletionAlgoTypeBagOfWords) names.push_back("bag_of_words");
  if (type & kCompletionAlgoTypeAlternateNames) names.push_back("alternate_names");
  if (type & kCompletionAlgoTypeSynonyms) names.push_back("synonyms");
  if (type & kCompletionAlgoTypeSpellCorrection) names.push_back("spelling");
  if (type & kCompletionAlgoTypeTemplateExpansion) names.push_back("template");
  if (type & kCompletionAlgoTypeAttribute) names.push_back("attribute");

  return strutil::JoinString(names, ",");
}

}  // namespace util
}  // namespace suggest
