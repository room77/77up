// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

#include "base/common.h"
#include "search_index.h"

FLAG_int(snippet_context_size_pre, 30,
    "Min number of characters to retain before the context.");
FLAG_int(snippet_context_size_post, 30,
    "Min number of characters to retain before the context.");
FLAG_int(snippet_merge_threshold, 40,
    "If two snippet's main tokens are closer than this many chars, merge.");
FLAG_double(close_proximity_score_multiplier, 2.0,
    "When search terms are in close proximity, ");
FLAG_int(max_query_length, 15,
    "We do not allow more than this many tokens in the query.");
FLAG_int(max_subsnippets, 5,
    "Do not retain more than this many subsnippets.");
FLAG_int(max_default_snippet_size, 300,
    "Default snippet will contain roughly this many characters.");
FLAG_bool(use_stemmer, true,
    "Enable stem based matching in search.");


namespace meta {

bool SearchIndex::StopWord(const string& term) {
  static unordered_set<string> stop_words {
    "a", "an", "of", "the",
    // Bigger stop word list if we ever need it.
    /*
    "a","about","above","after","again","against","all","am","an","and","any",
    "are","aren't","as","at","be","because","been","before","being","below",
    "between","both","but","by","can't","cannot","could","couldn't","did",
    "didn't","do","does","doesn't","doing","don't","down","during","each","few",
    "for","from","further","had","hadn't","has","hasn't","have","haven't",
    "having","he","he'd","he'll","he's","her","here","here's","hers","herself",
    "him","himself","his","how","how's","i","i'd","i'll","i'm","i've","if","in",
    "into","is","isn't","it","it's","its","itself","let's","me","more","most",
    "mustn't","my","myself","no","nor","not","of","off","on","once","only","or",
    "other","ought","our","ours","ourselves","out","over","own","same","shan't",
    "she","she'd","she'll","she's","should","shouldn't","so","some","such",
    "than","that","that's","the","their","theirs","them","themselves","then",
    "there","there's","these","they","they'd","they'll","they're","they've",
    "this","those","through","to","too","under","until","up","very","was",
    "wasn't","we","we'd","we'll","we're","we've","were","weren't","what",
    "what's","when","when's","where","where's","which","while","who","who's",
    "whom","why","why's","with","won't","would","wouldn't","you","you'd",
    "you'll","you're","you've","your","yours","yourself","yourselves"
    */
  };
  return stop_words.find(term) != stop_words.end();
}

vector<string> SearchIndex::ExpandTerm(const string& term) {
  struct Initializer {
    map<string, vector<string>> operator()() {
      static vector<vector<string>> synonyms {
        vector<string> { "free", "complimentary" },
        vector<string> { "tub", "bathtub" },
      };
      map<string, vector<string>> syn_map;
      for (int i = 0; i < synonyms.size(); ++i) {
        for (int j = 0; j < synonyms[i].size(); ++j) {
          for (int k = 0; k < synonyms[i].size(); ++k) {
            if (j != k)
              syn_map[NormalizeTerm(synonyms[i][j])].push_back(NormalizeTerm(synonyms[i][k]));
          }
        }
      }
      return syn_map;
    }
  };
  static map<string, vector<string>> syn_map = Initializer()();
  auto it = syn_map.find(term);
  if (it == syn_map.end()) return vector<string>();
  else return it->second;
}


string SearchIndex::RemoveHtmlTags(const string& s) {
  string out;
  for (int i = 0; i < s.size(); ++i) {
    char next;
    if (s[i] == '<') {
      next = ' ';
      for (; i < s.size() - 1 && s[i] != '>'; ++i) ;  // skip till '>' or end.
    }
    else next = s[i];
    if (next != ' ') out += next;
    else if (!out.empty() && *out.rbegin() != ' ') out += ' ';
  }
  auto pos = out.find('>');
  if (pos != string::npos) out = out.substr(pos+1);
  for (int i = 0; i < out.size(); ++i) {
    if (out[i] == '<' || out[i] == '>') out[i] = ' ';
  }
  return out;
}

string SearchIndex::HighlightTerms(const string& text, const set<string>& tokens,
    const string& begin_tag, const string& end_tag) {
  map<int, bool> positions;
  for (auto it = tokens.begin(); it != tokens.end(); ++it) {
    const string& token = *it;
    auto pos = text.find(token);
    while (pos != string::npos) {
      int size = token.size();
      if ((pos == 0 || !is_alphanumeric()(text[pos -1])) &&
        (pos + size == text.size() || !is_alphanumeric()(text[pos + size]))) {
        positions.insert(make_pair(pos, true));
        positions.insert(make_pair(pos + size, false));
      }
      pos = text.find(token, pos + size);
    }
  }
  string out;
  auto it = positions.begin();
  for (int i = 0; i < text.size(); ++i) {
    if (it != positions.end() && it->first == i) {
      out += it->second ? begin_tag : end_tag;
      ++it;
    }
    out += text[i];
  }
  if (it != positions.end()) out += end_tag;
  return out;
}

}
