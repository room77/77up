// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>

#include "meta/suggest/server/algos/util/suggest_algo_utils.h"

#include "util/string/stopwords/stopwords.h"
#include "util/templates/comparator.h"

FLAG_string(stopwords_id, "stopwords_small",
            "The stopwords id to use to create stopwords.");

namespace suggest {
namespace algo {

double GetWordMisMatchExtent(const string& str, const vector<string>& words, const string& lang) {
  // Check if this word is a stop word and can be ignored.
  i18n::StopWords::shared_proxy stopwords = i18n::StopWords::Create(gFlag_stopwords_id, lang);

  // Pair of <word query index, position in str>
  vector<pair<int, int>> word_positions;
  word_positions.reserve(words.size());
  double score = 0;
  int query_pos = 0;
  for (size_t i = 0; i < words.size(); ++i) {
    const string& word = words[i];
    size_t str_pos = str.find(word);
    if (str_pos != string::npos) {
      score += abs(static_cast<int>(str_pos) - query_pos) * word.size();
      word_positions.push_back(make_pair(i, str_pos));
    } else {
      if (stopwords->IsStopWord(word[word.size() - 1] == ' ' ?
          strutil::GetTrimmedString(word) : word)) {
        score += str.size() * word.size();
      } else {
        score = -1;
        break;
      }
    }
    query_pos += word.size();
  }

  // If we could not match critical words, do not consider this a match.
  if (score == -1) return score;

  sort(word_positions.begin(), word_positions.end(),
       ::util::tl::less_second<pair<int, int>>());

  double total_out_of_orders = 0;
  for (size_t i = 0; i < word_positions.size(); ++i)
    total_out_of_orders += abs(word_positions[i].first - static_cast<int>(i));

  score += total_out_of_orders / 2;

  return score;
}

}  // namespace algo
}  // namespace suggest
