// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

// Pretty optimized class that handles indexing, searching, scoring, and snippet
// generation.
//
// Once an instance is constructed and initialized, only const methods should be
// used to ensure thread safety.
//
// See search_index_test.cc for sample usage.

#ifndef _PUBLIC_UTIL_INDEX_SEARCH_INDEX_H_
#define _PUBLIC_UTIL_INDEX_SEARCH_INDEX_H_

#include "base/common.h"
#include "util/string/unicode.h"
#include "util/string/porter_stemmer.h"
#include "util/serial/serializer.h"
#include "util/cache/shared_lru_cache.h"
#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <queue>
#include <string>
#include <cmath>
#include <cctype>

extern int gFlag_snippet_context_size_pre;
extern int gFlag_snippet_context_size_post;
extern int gFlag_snippet_merge_threshold;
extern double gFlag_close_proximity_score_multiplier;
extern int gFlag_max_subsnippets;
extern int gFlag_max_query_length;
extern int gFlag_max_default_snippet_size;
extern bool gFlag_use_stemmer;

namespace meta {

class SearchIndex {
 public:
  typedef string::const_iterator iterator;

  // Support data structures.

  // Blob contains begin() and end() pointer for a blob of text, along with type
  // information. We support up to 256 types. In order to store the information
  // in a compact form, the class internally keeps tracks of each BlobInfo it
  // encounters and maps them to unique blob ids. This approach has two purposes
  // - Reduce memory footprint by storing multiple blob instances efficiently.
  // - Add a transparent indirection around raw pointers (iterators).
  // The latter allows loading of an optional index file rather than indexing
  // everything from scratch on server restart.
  class Blob {
    friend class SearchIndex;
   public:
    typedef uint32_t id_type;
    Blob() {}
    Blob(iterator begin, iterator end, unsigned char type) {
      BlobInfo b = { begin, end, type };
      auto it = BlobToId().find(b);
      if (it == BlobToId().end()) {
        id_ = BlobToId().size();
        BlobToId()[b] = id_;
        IdToBlob()[id_] = b;
        ASSERT(BlobToId().size() < numeric_limits<id_type>::max());
        ASSERT(IdToBlob().size() < numeric_limits<id_type>::max());
      } else {
        id_ = it->second;
      }
    }
    const iterator begin() const { return IdToBlob()[id_].begin_; }
    const iterator end() const { return IdToBlob()[id_].end_; }
    unsigned char type() const { return IdToBlob()[id_].type_; }
    bool operator<(const Blob& p) const {
      return make_pair(type(), begin()) < make_pair(p.type(), p.begin());
    }
    struct BlobInfo {
      iterator begin_;
      iterator end_;
      unsigned char type_;
      bool operator<(const BlobInfo& b) const {
        return make_pair(type_, begin_) < make_pair(b.type_, b.begin_);
      }
    };
   private:
    id_type id_;
    static map<BlobInfo, id_type>& BlobToId() {
      static map<BlobInfo, id_type> blob_to_id;
      return blob_to_id;
    }
    static unordered_map<id_type, BlobInfo>& IdToBlob() {
      static unordered_map<id_type, BlobInfo> id_to_blob;
      return id_to_blob;
    }
   public:
    SERIALIZE(id_*1);
  };

  // Somewhat compact way to represent ranges. Data structure occupies 4 bytes
  // in 32 bits systems and is suitable for dynamic on the fly computations.
  // About half the size of a comparable naive implementation.
  class Range {
   public:
    typedef pair<unsigned short int, unsigned short int> id_type;

    Range(const Blob& b, iterator begin, iterator end) {
      id_ = pair<int, int>(begin - b.begin(), end - b.begin());
    }

    Range(const id_type& id) : id_(id) {}

    const iterator begin(iterator begin) const  { return begin + id_.first; }
    const iterator begin(const Blob& b) const   { return b.begin() + id_.first; }
    const iterator end(iterator begin) const    { return begin + id_.second; }
    const iterator end(const Blob& b) const     { return b.begin() + id_.second; }
    const id_type id() const                    { return id_; }
    bool operator<(const Range& r) const { return id_ < r.id_; }

   private:
    id_type id_;
  };

  // CompactRange contains begin and end information for each interesting "term"
  // within a Blob. This data structure occupies 2 bytes in 32 bits systems.
  // Blob must be passed since CompactRanges are relative to the Blob. Blob size
  // must not be greater than 64K-1. In order to store the information in a
  // compact form, the class internally keeps tracks of each unique offset /
  // size pairs it encounters (which must not exceed 64K) and maps them to
  // unique range ids. This approach allows us to cut the memory footprint by
  // more than half compared to a naive implementation. Transparent to end user.
  class CompactRange {
    friend class SearchIndex;
   public:
    typedef uint32_t id_type;

    CompactRange() {}
    CompactRange(const Blob& b, iterator begin, iterator end) {
      pair<int, int> p(begin - b.begin(), end - b.begin());
      auto it = RangeToId().find(p);
      if (it == RangeToId().end()) {
        id_ = RangeToId().size();
        RangeToId()[p] = id_;
        IdToRange()[id_] = p;
        ASSERT(RangeToId().size() < numeric_limits<id_type>::max());
        ASSERT(IdToRange().size() < numeric_limits<id_type>::max());
      } else {
        id_ = it->second;
      }
    }

    operator Range() const { return Range(IdToRange()[id_]); }

    const iterator begin(iterator begin) const  { return begin + IdToRange()[id_].first; }
    const iterator begin(const Blob& b) const   { return b.begin() + IdToRange()[id_].first; }
    const iterator end(iterator begin) const    { return begin + IdToRange()[id_].second; }
    const iterator end(const Blob& b) const     { return b.begin() + IdToRange()[id_].second; }
    id_type id() const                          { return id_; }
    bool operator<(const CompactRange& r) const { return IdToRange()[id_] < IdToRange()[r.id_]; }

   private:
    id_type id_;
    static map<pair<int,int>, id_type>& RangeToId() {
      static map<pair<int,int>, id_type> range_to_id;
      return range_to_id;
    }
    static unordered_map<id_type, pair<int, int>>& IdToRange() {
      static unordered_map<id_type, pair<int, int>> id_to_range;
      return id_to_range;
    }
   public:
    SERIALIZE(id_*1);
  };

  // LocationInfo stores the locations of occurences of a term within a blob of
  // text along with minimal information for the Blob.
  //
  // For reduced memory footprint, rather than allocating many many tiny
  // vector<CompactRange>s to store location information, LocationInfo
  // internally pushes the ranges into a contiguous deque and stores offsets and
  // size information instead. This reduces the memory overhead to about 2 bytes
  // per range (we have 10s of millions of ranges overall).
  // Deque is particularly suitable for this purpose since, unlike vector,
  // it does not require resizing and does not waste much memory.
  class LocationInfo {
    friend class SearchIndex;
   public:
    LocationInfo() {}
    LocationInfo(const Blob& b, const vector<CompactRange>& ranges)
        : blob_(b) {
      range_offset_ = RangeStorage().size();
      range_size_ = ranges.size();
      RangeStorage().insert(RangeStorage().end(), ranges.begin(), ranges.end());
    }
    const Blob& blob() const              { return blob_; }
    unsigned short int size() const       { return range_size_; }
    const CompactRange& operator[](int i) const  { return RangeStorage()[range_offset_ + i]; }

   private:
    static deque<CompactRange>& RangeStorage() {
      static deque<CompactRange> ranges;
      return ranges;
    }
    Blob blob_;
    int range_offset_;
    unsigned short int range_size_;
   public:
    SERIALIZE(blob_*1 / range_offset_*2 / range_size_*3);
  };

  typedef map<string, const vector<LocationInfo>*> MatchType;

  // Helper functions.
  template<class Iter, class Match, class Func>
  static void for_each_range_of(Iter begin, Iter end, Match m, Func f) {
    for (auto it = begin; it != end;) {
      for (; !m(*it) && it != end; ++it) ;
      for (begin = it; m(*it) && it != end; ++it) ;
      if (begin < it) f(begin, it);
    }
  }

  template<class Container, class Match, class Func>
  static void for_each_range_of(const Container& c, Match m, Func f) {
    for_each_range_of(c.begin(), c.end(), m, f);
  }

  template<class Iter, class Match, class Func>
  static void for_each_range_not_of(Iter begin, Iter end, Match m, Func f) {
    for (auto it = begin; it != end;) {
      for (; m(*it) && it != end; ++it) ;
      for (begin = it; !m(*it) && it != end; ++it) ;
      if (begin < it) f(begin, it);
    }
  }

  template<class Container, class Match, class Func>
  static void for_each_range_not_of(const Container& c, Match m, Func f) {
    for_each_range_not_of(c.begin(), c.end(), m, f);
  }

  static string LowerCase(const string& s) {
    static unicode::Lower lowercase;
    string lower = lowercase(s);
    if (s.size() == lower.size()) return lower;
    else {
      VLOG(2) << "Lowercase of text is of different size. Will not lowercase!\n " << s;
      return s;
    }
  }

  // Tokenize a given string into individual terms.
  template<class Container = vector<string>>
  static Container Tokenize(const string& text) {
    Container tokens;
    for_each_range_of(text, is_alphanumeric(), inserter<Container>(tokens));
    return tokens;
  }

  // Same as Tokenize, but stems the terms as well.
  template<class Container = vector<string>>
  static Container NormalizeTokenize(const string& text) {
    Container tokens;
    for_each_range_of(text, is_alphanumeric(), normalize_inserter<Container>(tokens));
    return tokens;
  }

  // Removes HTML tags fromt the string. Fairly basic implementation.
  // Returns the resulting "html-free" string.
  static string RemoveHtmlTags(const string& s);

  // Highlights search terms (and potentially synonyms and stems).
  static string HighlightTerms(const string& text, const set<string>& terms,
      const string& begin_tag = "<b>", const string& end_tag = "</b>");

  static bool StopWord(const string& term);

  static vector<string> ExpandTerm(const string& term);

  // Returns true if the token is indexable.
  bool Indexable(const string& token) const {
    return token.size() > 1 && !StopWord(token);
  }

  // Returns true if the token can be searched.
  static bool Searchable(const string& token) {
    // Make everything that we indexed searchable for now.
    return true;
    //return token.size() > 3 || Idf(token) > 0.5;
  }

  // Helper functors.
  // Some of these can be implemented much more easily using C++0x lambda syntax
  // but since we got stuck with an older gcc version, we need to wait till the
  // next LTS version of Ubuntu. Once we do, the code can be made more compact.
  struct is_alphanumeric {
    bool operator()(char c) {
      return 'a' <= c && c <= 'z' ||
             'A' <= c && c <= 'Z' ||
             '0' <= c && c <= '9' ||
             c == '-' || c == '\'';
    }
  };

  struct is_not_alphanumeric {
    bool operator()(char c) {
      return !is_alphanumeric()(c);
    }
  };

  template<class T>
  struct inserter {
    inserter(T& tokens) : tokens_(tokens) {}
    void operator()(iterator& begin, iterator& end) {
      tokens_.insert(tokens_.end(), string(begin, end));
    }
    T& tokens_;
  };

  template<class T>
  struct normalize_inserter {
    normalize_inserter(T& tokens) : tokens_(tokens) {}
    void operator()(iterator& begin, iterator& end) {
      tokens_.insert(tokens_.end(), NormalizeTerm(begin, end));
    }
    T& tokens_;
  };

  // Public methods.

  // Return the IDF of the term.
  float Idf(const string& term) const {
    auto it = term_freq_.find(LowerCase(term));
    if (it == term_freq_.end()) return 0.01;
    return 0.01 + log(doc_ids_.size() / it->second.size());
  }

  // Return the TF of the term for document i.
  float Tf(const string& term, int i) {
    ASSERT(false) << "Not implemented yet.";
    return 0;
  }

  map<unsigned char, string>& TypeIdToName() const {
    static map<unsigned char, string> type_id_to_name;
    return type_id_to_name;
  }

  // Returns the type id of the part of the document. Automatically creates
  // unique type ids for each unique "name" it has seen.
  unsigned char TypeId(const string& name) {
    static map<string, unsigned char> type_name_to_id;
    auto it = type_name_to_id.find(name);
    if (it == type_name_to_id.end()) {
      unsigned char id = type_name_to_id.size() + 1;
      type_name_to_id[name] = id;
      TypeIdToName()[id] = name;
    }
    return type_name_to_id[name];
  }

  // Returns a mutable ref to the weight of the type id.
  double& TypeWeight(const string& name) {
    static unordered_map<int, double> type_weight;
    return type_weight[TypeId(name)];
  }

  // Updates the IDF of the term. Useful only if IDFs are not updated during
  // indexing.
  void UpdateIdf(int i, const string& text) {
    vector<string> terms = NormalizeTokenize(text);
    for (int t = 0; t < terms.size(); ++t) {
      string term = LowerCase(terms[t]);
      ++term_freq_[term][i];
      doc_ids_.insert(i);
    }
  }

  struct blob_indexer {
    blob_indexer(SearchIndex& caller, Blob b, map<string, vector<CompactRange>>& index)
      : caller_(caller), blob_(b), index_(index) {}
    void operator()(iterator begin, iterator end) {
      auto term = caller_.NormalizeTerm(begin, end);
      if (caller_.Indexable(term)) index_[term].push_back(CompactRange(blob_, begin, end));
    }
    SearchIndex& caller_;
    Blob blob_;
    map<string, vector<CompactRange>>& index_;
  };

  // Index a whole chunk for hotel with type name and blob of text. Text is
  // going to be chopped into indexable "terms".
  void IndexBlob(int i, const string& type_name, const string& text) {
    unsigned char type = TypeId(type_name);
    Blob b { text.begin(), text.end(), type };

    // If the index is loaded from a file, we just need to construct the
    // type ids and blobs. Return here.
    if (index_blob_only_) return;

    UpdateIdf(i, text);
    map<string, vector<CompactRange>> index;
    for_each_range_of(text, is_alphanumeric(), blob_indexer(*this, b, index));
    for (auto it = index.begin(); it != index.end(); ++it) {
      index_[it->first][i].push_back(LocationInfo(b, it->second));
    }
  }

  static string NormalizeTerm(const string& term) {
    static PorterStemmer stemmer;
    if (gFlag_use_stemmer) return stemmer.StemTerm(LowerCase(term));
    else return LowerCase(term);
  }

  static string NormalizeTerm(iterator begin, iterator end) {
    static PorterStemmer stemmer;
    if (gFlag_use_stemmer) return stemmer.StemTerm(LowerCase(string(begin, end)));
    else return LowerCase(string(begin, end));
  }

  map<string, vector<CompactRange>> BlobIndex(const string& type_name, const string& text) {
    unsigned char type = TypeId(type_name);
    Blob b { text.begin(), text.end(), type };
    map<string, vector<CompactRange>> index;
    for_each_range_of(text, is_alphanumeric(), blob_indexer(*this, b, index));
    return index;
  }

  static void DumpBlobIndex(const map<string, vector<CompactRange>>& index, const string& text) {
    Blob b { text.begin(), text.end(), 0 };
    for (auto it = index.begin(); it != index.end(); ++it) {
      cout << it->first;
      for (int i = 0; i < it->second.size(); ++i) {
        const CompactRange& r = it->second[i];
        cout << " " << string(r.begin(b), r.end(b));
      }
      cout << endl;
    }
  }

  // Perform a search on selected ids, for the given query q.
  // This method should only be used once all data is initialized.
  vector<MatchType> Search(const vector<int>& ids, const vector<string>& terms) const {
    vector<MatchType> matches(ids.size());
    for (int i = 0; i < ids.size(); ++i) {
      for (int t = 0; t < terms.size(); ++t) {
        if (!Searchable(terms[t])) continue;
        auto it = index_.find(terms[t]);
        if (it == index_.end()) continue;
        auto jt = it->second.find(ids[i]);
        if (jt != it->second.end()) {
          matches[i][terms[t]] = &jt->second;
        }
      }
    }
    return matches;
  }

  map<Blob, map<Range, float>> GenerateCandidateMatches(const MatchType& matches) const {
    map<Blob, map<Range, float>> candidates;
    for (auto it = matches.begin(); it != matches.end(); ++it) {
      const string& term = it->first;
      ASSERT(it->second != nullptr);
      const vector<LocationInfo>& li_vec = *it->second;
      for (int i = 0; i < li_vec.size(); ++i) {
        const LocationInfo& li = li_vec[i];
        const Blob& b = li.blob();
        for (int j = 0; j < li.size(); ++j) {
          candidates[b][li[j]] = Idf(term);
        }
      }
    }

    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      for (auto jt = it->second.begin(); jt != it->second.end();) {
        auto kt = jt;
        if (++kt == it->second.end()) break;
        const Blob& b = it->first;
        if (kt->first.begin(b) > jt->first.end(b) &&
            kt->first.begin(b) - jt->first.end(b) < gFlag_snippet_merge_threshold) {
          vector<string> tokens = Tokenize(string(jt->first.end(b), kt->first.begin(b)));
          if (tokens.size() <= 4) {
            auto tmp = make_pair(Range(b, jt->first.begin(b), kt->first.end(b)),
                                 (jt->second + kt->second) * gFlag_close_proximity_score_multiplier);
            it->second.erase(kt);
            it->second.erase(jt);
            jt = it->second.insert(tmp).first;
          }
          else ++jt;
        }
        else ++jt;
      }
    }
    return candidates;
  }

  float Score(const map<Blob, map<Range, float>>& candidates) const {
    float score = 1;
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
        score += jt->second;
      }
    }
    return score;
  }

  float Score(const MatchType& matches) const {
    return Score(GenerateCandidateMatches(matches));
  }

  string GenerateSubSnippet(const Blob& b, Range r, double score) const {
    auto begin = r.begin(b) - gFlag_snippet_context_size_pre;
    if (begin < b.begin()) begin = b.begin();
    auto end = r.end(b) + gFlag_snippet_context_size_post;
    if (end > b.end()) end = b.end();

    // Complement the full token at the beginning/end that might have been chopped.
    for (auto it = begin; it >= b.begin() && is_alphanumeric()(*it); --it) begin = it;
    for (auto it = end; it < b.end() && is_alphanumeric()(*it); ++it) end = it+1;

    // Find closest estimated sentence boundary on both sides if any and break
    // there. For now we look at ". " and "<br".
    for (auto it = begin; it < r.begin(b)-2; ++it) {
      if (*it == '.' && *(it+1) == ' ') begin = it+2;
      if (*it == '<' && *(it+1) == 'b' && *(it+2) == 'r') begin = it+3;
    }
    for (auto it = end-3; it >= r.end(b); --it) {
      if (*it == '.' && *(it+1) == ' ') end = it+1;
      if (*it == '<' && *(it+1) == 'b' && *(it+2) == 'r') end = it+2;
    }
    return string(begin, end);
  }

  set<string> UniqueTermInstances(const MatchType& matches) const {
    set<string> unique_terms;
    for (auto it = matches.begin(); it != matches.end(); ++it) {
      ASSERT(it->second != nullptr);
      const vector<LocationInfo>& li_vec = *it->second;
      for (int i = 0; i < li_vec.size(); ++i) {
        const LocationInfo& li = li_vec[i];
        const Blob& b = li.blob();
        for (int j = 0; j < li.size(); ++j) {
          unique_terms.insert(string(li[j].begin(b), li[j].end(b)));
        }
      }
    }
    return unique_terms;
  }

  // Generate a snippet based on the match.
  string GenerateSnippet(const MatchType& matches, const vector<string>& terms) const {
    map<Blob, map<Range, float>> candidates = GenerateCandidateMatches(matches);
    return GenerateSnippet(matches, candidates, terms);
  }

  string GenerateSnippet(const MatchType& matches, const map<Blob, map<Range, float>>& candidates, const vector<string>& terms) const {
    set<string> unique_terms;
    for (auto it = matches.begin(); it != matches.end(); ++it) {
      ASSERT(it->second != nullptr);
      const vector<LocationInfo>& li_vec = *it->second;
      for (int i = 0; i < li_vec.size(); ++i) {
        const LocationInfo& li = li_vec[i];
        const Blob& b = li.blob();
        for (int j = 0; j < li.size(); ++j) {
          unique_terms.insert(string(li[j].begin(b), li[j].end(b)));
        }
      }
    }

    priority_queue<float> scores;
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
        scores.push(jt->second);
      }
    }
    map<float, int> score_limits;
    for (int i = 0; i < gFlag_max_subsnippets && !scores.empty(); ++i) {
      ++score_limits[scores.top()];
      scores.pop();
    }

    set<string> subsnippets;
    string snippet;
    unsigned char type = 255;
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
        if (--score_limits[jt->second] < 0) continue;
        const Blob& b = it->first;
        const Range& r = jt->first;
        if (b.type() != type) {
          if (type != 255) snippet += "</p>";
          snippet += "<p>";
          type = b.type();
          snippet += "<em>" + TypeIdToName()[type] + ":</em>";
        }
        unique_terms.insert(string(r.begin(b), r.end(b)));
        string subsnippet = HighlightTerms(RemoveHtmlTags(
            GenerateSubSnippet(b, Range(b, r.begin(b), r.end(b)), jt->second)),
            unique_terms);
        if (subsnippets.insert(subsnippet).second)
          snippet += " ... " + subsnippet;
      }
    }
    if (snippet.empty()) return snippet;
    snippet += " ... </p>";
    return snippet;
  }

  string GenerateDefaultSnippet(const string& text) const {
    // Disabled for now.
    return "";

    int size = text.size();
    if (size > gFlag_max_default_snippet_size) size = gFlag_max_default_snippet_size;
    string snippet = RemoveHtmlTags(text.substr(0, size));
    auto pos = snippet.find_last_of(". ");
    if (pos != string::npos) snippet.resize(pos);
    else {
      for (int i = pos; i > 0 && is_alphanumeric()(snippet[i]); --i) size = i;
      snippet.resize(size);
    }
    pos = snippet.find_first_of(". ");
    if (pos != string::npos && pos < 10) snippet = snippet.substr(pos + 2);
    snippet += " ...";
    return "<em>Description:</em> " + snippet;
  }

  static vector<string> ProcessQuery(const string& q) {
    vector<string> tokens = NormalizeTokenize(LowerCase(q));
    if (tokens.size() > gFlag_max_query_length)
      tokens.resize(gFlag_max_query_length);
    vector<string> good_tokens;
    for (int i = 0; i < tokens.size(); ++i) {
      if (Searchable(tokens[i])) {
        good_tokens.push_back(tokens[i]);
        vector<string> exp = ExpandTerm(tokens[i]);
        good_tokens.insert(good_tokens.end(), exp.begin(), exp.end());
      }
    }
    return good_tokens;
  }

  string GenerateSnippet(int id, const vector<string>& terms) const {
    auto search_results = Search(vector<int>(1, id), terms);
    if (search_results.size() == 1) return GenerateSnippet(search_results[0], terms);
    else return "";
  }

  void DumpContents() {
    for (auto it = index_.begin(); it != index_.end(); ++it) {
      cout << it->first << " (" << Idf(it->first) << ")" << endl;
      for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
        int id = jt->first;
        cout << "\t" << id << "\t";
        for (auto i = 0; i < jt->second.size(); ++i) {
          const LocationInfo& ri = jt->second[i];
          const Blob& b = ri.blob();
          for (int j = 0; j < ri.size(); ++j) {
            cout << " " << string(ri[i].begin(b), ri[i].end(b));
          }
        }
        cout << endl;
      }
    }
  }

  // Finalize the index and check invariants (useful if index was loaded).
  // Must be called after all calls to IndexBlob() and LoadIndex(), and before
  // any other calls that uses the index (including SaveIndex())
  void Finalize() {
    if (index_blob_only_) {
      ASSERT_EQ(expected_num_blobs_, Blob::BlobToId().size())
          << "Number of blobs expected does not match number of blobs found. "
          << "Index data file may be corrupt or stale. This may typically "
          << "happen if one or more data files under /home/share/data/search/ "
          << "is updated after the index file is generated (or \"current\" "
          << "symlinks point to other version(s) now). Index file needs to be "
          << "regenerated.";
    }
    expected_num_blobs_ = Blob::BlobToId().size();
    index_blob_only_ = false;
    done_ = true;
  }

  // This call should not happen while any threads are accesing the index.
  // Reopens the index after Finalize() has been called. For additional
  // on-the-fly indexing. Finalize() must be called again once done.
  void Reopen() {
    ASSERT(done_);
    index_blob_only_ = false;
    done_ = false;
  }

  // Load index from a precomputed index file.
  bool LoadIndex(const string& filename) {
    ASSERT(!done_);
    ifstream file(filename);
    if (file.fail()) return false;
    LOG(INFO) << "Reading index from " << filename;
    serial::Serializer::FromRawBinary(file, this);
    index_blob_only_ = true;
    return true;
  }

  // Write index to an index file.
  bool SaveIndex(const string& filename) {
    ASSERT(done_);
    ofstream file(filename);
    LOG(INFO) << "Writing index to " << filename;
    serial::Serializer::ToRawBinary(file, *this);
    return true;
  }

  // Main index data structure.
  // term -> docid -> occurence information for each blob.
  unordered_map<string, unordered_map<int, vector<LocationInfo>>> index_;

  // Unique documents we have seen.
  unordered_set<int> doc_ids_;

  // term -> document -> count
  unordered_map<string, unordered_map<int, int>> term_freq_;

  int expected_num_blobs_ = 0;
  bool index_blob_only_ = false;
  bool done_ = false;

  // Do not use serialization methods explicitly. Use SaveIndex() / LoadIndex().
  SERIALIZE(index_*1 / doc_ids_*2 / term_freq_*3 / LocationInfo::RangeStorage()*4 / CompactRange::RangeToId()*5 /
            CompactRange::IdToRange()*6 / expected_num_blobs_*7);
};

}

#endif  // _PUBLIC_UTIL_INDEX_SEARCH_INDEX_H_
