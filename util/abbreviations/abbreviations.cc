#include "util/abbreviations/abbreviations.h"

#include <stack>
#include "util/region_data/utils.h"

namespace util { namespace abbr {

// lists all possible combinations of the given phrase
// eg. "st ft" => ["st ft", "st fort", "saint ft", "saint fort"]
// @param phrase - assumed normalized
// @return - vector of normalized variants (abbreviation <-> completion)
vector<string> GetAllVariants(const string& phrase, const string& delim) {
  using region_data::utils::NormalizeString;

  struct GetWordVariants {
    vector<string> operator()(const string& word) {
      static Abbreviation::shared_proxy abbr =
          Abbreviation::make_shared("name_abbr");
      ASSERT_NOTNULL(abbr);

      vector<string> ret;
      ret.push_back(word);
      if (abbr->IsAbbreviation(word)) {
        ret.push_back(NormalizeString(abbr->GetCompletion(word)));
      }
      if (abbr->IsCompletion(word)) {
        ret.push_back(NormalizeString(abbr->GetAbbreviation(word)));
      }
      return ret;
    }
  };

  struct HasVariants {
    bool operator()(const vector<string>& words) {
      static Abbreviation::shared_proxy abbr =
          Abbreviation::make_shared("name_abbr");
      ASSERT_NOTNULL(abbr);

      for (const string& word : words) {
        if (abbr->IsAbbreviation(word)) return true;
        if (abbr->IsCompletion(word)) return true;
      }
      return false;
    }
  };

  vector<string> ret;

  vector<string> words;
  strutil::SplitString(phrase, delim, &words);

  if (words.empty()) {
    ASSERT_DEV(false);
    return ret;
  }

  // Perhaps in the vast majority cases, there is no other variants.
  // Let's optimize based on this case
  if (!HasVariants()(words)) {
    ret.push_back(phrase);
    return ret;
  }

  stack<pair<int, string>> s;
  s.push(make_pair(-1, ""));

  vector<string> scratch_board(words.size());
  while (!s.empty()) {
    const auto& p = s.top();
    const int pos = p.first;
    ASSERT_LT(pos, static_cast<int>(words.size()));
    const string word = p.second;
    s.pop();

    if (pos >= 0) scratch_board[pos] = word;
    if (pos == words.size() - 1) {
      // terminal case
      ret.push_back(strutil::Join(scratch_board, " "));
    } else {
      for (const string& word_variant : GetWordVariants()(words[pos + 1])) {
        s.push(make_pair(pos + 1, word_variant));
      }
    }
  }
  return ret;
}

} }  // namespace util::abbr
