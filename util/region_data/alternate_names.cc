#include "util/region_data/alternate_names.h"

#include "util/region_data/region.h"
#include "util/region_data/synonyms.h"
#include "util/region_data/utils.h"

namespace region_data { namespace alternate_names {

namespace {

// assumed word is in lowercase
bool IsStopWord(const string& word) {
  static const unordered_set<string> stop_words {
    "de", "na", "el", "la", "di", "da", "le", "les", "ko", "koh"
      /*, "si", "al", "pa", "ta" */
  };
  return (word.size() == 2 || word.size() == 3)
      && ::util::tl::Contains(stop_words, word);
}

// NOTE assumed that s is normalized
string Expand(const string& s) {
  static auto abbr = util::abbr::Abbreviation::make_shared("name_abbr");
  return abbr->ReplaceAllAbbreviations(s);
}

// @param - normalized
string Abbreviate(const string& name) {
  static auto abbr = util::abbr::Abbreviation::make_shared("name_abbr");
  ASSERT_NOTNULL(abbr);
  return abbr->ReplaceAllCompletions(name);
}

// @param - MUST NOT BE NORMALIZED!
// @return - not normalized
string SkipApostrophe(const string& s) {
  string ret; ret.reserve(s.size());
  for (const char c : s) if (c != '\'') ret.push_back(c);
  return ret;
}

// inserts the version without stop words ONLY
// @param name - normalized
void AlterByStopWord(const string& name, unordered_set<string> *p_res) {
  const string non_stop_words_only = ExtractNonStopWords(name);
  if (!non_stop_words_only.empty()) p_res->insert(non_stop_words_only);
}

// inserts BOTH versions of synonyms
// @param name - normalized
void AlterBySynonym(const string& name, unordered_set<string> *p_res) {
  // Populate all variants of synonyms.
  // In the vast majority of cases, city names don't have synonyms
  static const auto& synonyms = region_data::Synonyms::Instance();
  const auto variants = synonyms.GetAllVariants(name, false);
  p_res->insert(variants.begin(), variants.end());
}

using AltererType = function<void (const string&, unordered_set<string>*)>;

// apply alterer over existing alternate names
void Apply(const AltererType& alterer, unordered_set<string> *p_res) {
  vector<const string *> variants; variants.reserve(p_res->size());
  for (const auto& variant : *p_res) variants.push_back(&variant);
  for (const auto *p_variant : variants) {
    alterer(*p_variant, p_res);
  }
}

// all region names should go through this
void GetAbbreviatedNames(const tRegion& region, const string& normalized_name,
                         unordered_set<string> *p_res) {
  vector<string> known_names;
  region.get_alternate_names(&known_names);

  p_res->insert(Abbreviate(normalized_name));
  for (const string& name : known_names) p_res->insert(Abbreviate(name));
}

// e.g. ExtractParenthesized("Ab x (Ps L)", 5) == {"Ab x", "Ps L"}
// NOTE we don't need to be concerned about repeated spaces as the results
// will be normalized later.
void ExtractParenthesized(const string& name, size_t pos, vector<string> *res) {
  ASSERT_NE(pos, string::npos);
  ASSERT_GT(pos, 0);
  ASSERT_EQ(name[pos], '(');

  res->reserve(2);
  const auto right_paren = name.find_last_of(")");
  ASSERT(right_paren == string::npos || pos < right_paren);

  if (right_paren == string::npos) {
    res->push_back(name.substr(0, pos));
    res->push_back(name.substr(pos + 1));
  } else {
    const auto trailing = " " + name.substr(right_paren + 1);
    // up to pos -1, and then trailing
    res->push_back(name.substr(0, pos) + trailing);

    // from pos + 1 to right_paren - 1, and then trailing
    res->push_back(name.substr(pos + 1, right_paren - pos - 1) + trailing);
  }
  ASSERT(res->size() == 2 && !(*res)[1].empty());
}

}  // namespace unnamed

// TODO REMOVE and use the one in util/region_data/utils
// assumed orig is in lowercase
int ExtractNonStopWords(const string& orig, string *p_ret) {
  vector<string> words;
  strutil::SplitString(orig, " ", &words);

  int num_non_stop_words = 0;
  bool first = true;
  for (const string& word : words) {
    if (IsStopWord(word)) continue;
    if (!first) *p_ret += " ";
    first = false;
    *p_ret += word;
    ++num_non_stop_words;
  }
  return num_non_stop_words;
}

string ExtractNonStopWords(const string& orig) {
  string ret;
  ExtractNonStopWords(orig, &ret);
  return ret;
}

// NOTE the city input already contains the alternate city names;
// this function populates and writes back some derived alternate city names
// altering by abbreviation, stop words, and synonyms
void RewriteAlternateCityNames(tCity *p_city) {
  unordered_set<string> alternate_names_set;

  const string normalized_name = utils::NormalizeString(p_city->name);
  GetAbbreviatedNames(*p_city, normalized_name, &alternate_names_set);

  Apply(AlterByStopWord, &alternate_names_set);
  Apply(AlterBySynonym, &alternate_names_set);

  // now we can remove the primary (non-alternate) name
  alternate_names_set.erase(normalized_name);

  set<string> alternate_names_sorted(alternate_names_set.begin(),
                                     alternate_names_set.end());
  p_city->alternate_names = strutil::JoinString(alternate_names_sorted, "|");
}

// The core normalization commonly used for region data generation
// In addition to the usual normalization, it expands, and replaces synonyms
string NormalizeName(const string& name) {
  const auto expanded = Expand(utils::NormalizeString(name));
  const auto& synonyms = region_data::Synonyms::Instance();
  return synonyms.ReplaceWithSynonym(expanded, true);
}

// should be called right after parsing from the data source
void WriteNormalizedAndAlternateCityNames(tCity *p_region) {
  // NOTE this may be equal to the simple normalized name; we need to assure
  // that such cases should be handled before writing to files.
  p_region->AddAlternateName(
      utils::NormalizeString(SkipApostrophe(p_region->name)));
  p_region->normalized_name = NormalizeName(p_region->name);
}

// handle parentheses using ExtractParenthesized()
void WriteNormalizedAndAlternateAttractionNames(tAttraction *p_region) {
  vector<string> names; names.reserve(2);
  const auto pos = p_region->name.find("(");
  if (pos == string::npos) {
    names.push_back(p_region->name);
  } else {
    ExtractParenthesized(p_region->name, pos, &names);
  }

  set<string> alternate_names;

  for (const auto& name : names) {
    alternate_names.insert(utils::NormalizeString(name));
    alternate_names.insert(utils::NormalizeString(SkipApostrophe(name)));
  }

  p_region->normalized_name = utils::NormalizeString(p_region->name);
  alternate_names.erase(p_region->normalized_name);

  // sometimes attraction name fiels contains the city name in parentheses
  // e.g., Big Ben (London)
  // we must not allow a city name as an alternate name
  alternate_names.erase(utils::NormalizeString(p_region->city));

  p_region->alternate_names = strutil::JoinString(alternate_names, "|");
}

}  // namespace alternate_names
}  // namespace region_data
