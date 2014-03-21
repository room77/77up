// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_ALGOS_UTIL_SUGGEST_ALGO_UTILS_H_
#define _META_SUGGEST_SERVER_ALGOS_UTIL_SUGGEST_ALGO_UTILS_H_

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"

namespace suggest {
namespace algo {

// Returns the word mismatch extent between a string and a 'query'
// (an ordered list of words).
// Note that the score is not symmetric. 'str' is usually the superset string of
// 'query'. The higher the returned value the more is the word match.
// In case the two strings have nothing in common '-1' is returned.
// Note: we assume that str and query are already normalized using
// 'NormalizeString' function (or similar) if necessary.
// We compute the match based on two factors.
// 1. The positional offset of a word in the query with respect to the original.
// 2. The partial order of occurrence of words. If they occur in the same order
//    as in the query, they are scored better. e.g. for str [abc def ghi], the
//    query [def ghi] will be considered a better match as compared to [ghi def]
double GetWordMisMatchExtent(const string& str, const vector<string>& words,
                             const string& lang = "en");

// Utility function to compute word mismatch between a string and a query.
inline double GetWordMisMatchExtent(const string& str, const string& query,
                                    const string& lang = "en") {
  // Split words and find word mismatch.
  vector<string> words;
  strutil::SplitString(query, " ", &words);
  // Add the space back for all intermediate words.
  for (int i = 0; i < words.size() - 1; ++i) words[i] += " ";
  return GetWordMisMatchExtent(str, words);
}

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_UTIL_SUGGEST_ALGO_UTILS_H_
