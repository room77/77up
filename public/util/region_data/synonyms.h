// Copyright 2013 Room77, Inc.
// Author: Sungsoon Cho

#ifndef _UTIL_REGION_DATA_SYNONYMS_H_
#define _UTIL_REGION_DATA_SYNONYMS_H_

#include <map>

#include "base/defs.h"
#include "util/templates/container_util.h"

namespace region_data {

class Synonyms {
 public:
  static const Synonyms& Instance() { static Synonyms _; return _; }

  // @phrase - normalized
  string ReplaceWithSynonym(string phrase, bool preferred_only) const {
    vector<string> words;
    strutil::SplitString(phrase, " ", &words);
    bool replaced = false;
    for (auto& word: words) {
      const auto it = s_.find(word);
      if (it != s_.end() && (!preferred_only || it->second.second)) {
        word = it->second.first;
        replaced = true;
      }
    }
    if (replaced) phrase = strutil::JoinString(words, " ");
    return phrase;
  }

  bool HasSynonym(const string& phrase) const {
    vector<string> words;
    strutil::SplitString(phrase, ", ", &words);
    for (const auto& word : words) {
      if (::util::tl::Contains(s_, word)) return true;
    }
    return false;
  }

  bool Synonymous(const string& c1, const string& c2) const {
    return ReplaceWithSynonym(c1, true) == ReplaceWithSynonym(c2, true);
  }

  // TODO
  unordered_set<string> GetAllVariants(const string& phrase,
                                       bool include_self = true) const {
    unordered_set<string> ret;
    if (include_self) ret.insert(phrase);
    ret.insert(ReplaceWithSynonym(phrase, false));
    return ret;
  }

 private:
  // NOTE for now, we have only one pair. Move to a file later.
  Synonyms() {
    // the latter one is preferred
    static const vector<pair<string, string>> pairs {
      {"ko", "koh"}
    };

    for (const auto& p : pairs) {
      if (p.first == p.second) {
        ASSERT_DEV(false) << "identical synonym pair";
        continue;
      }
      s_.insert(make_pair(p.first, make_pair(p.second, true)));
      s_.insert(make_pair(p.second, make_pair(p.first, false)));
    }
  }

  unordered_map<string, pair<string, bool>> s_;
};

}  // namespace region_data

#endif

