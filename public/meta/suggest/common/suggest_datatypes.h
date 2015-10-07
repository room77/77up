// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The datatypes for complete suggestions.


#ifndef _META_SUGGEST_COMMON_SUGGEST_DATATYPES_H_
#define _META_SUGGEST_COMMON_SUGGEST_DATATYPES_H_

// Note: Be very careful about adding headers here.
// This file has a lot of dependencies.
// Whenever possible, use forward declaration to avoid including headers.
#include "base/defs.h"
#include "util/log/channel/channel.h"
#include "util/entity/entity_type.h"
#include "util/serial/serializer.h"
#include "util/thread/counters.h"
#include "util/thread/thread_pool.h"
#include "util/memory/unique.h"

namespace suggest {

enum {
  kCompletionAlgoTypeInvalid = 0,
  kCompletionAlgoTypePrefix = 1,
  kCompletionAlgoTypeMidString = 1 << 2,
  kCompletionAlgoTypeBagOfWords = 1 << 3,
  kCompletionAlgoTypeAlternateNames = 1 << 4,
  kCompletionAlgoTypeSynonyms = 1 << 5,
  kCompletionAlgoTypeSpellCorrection = 1 << 6,
  kCompletionAlgoTypeTemplateExpansion = 1 << 7,
  kCompletionAlgoTypeAttribute = 1 << 8,
};
typedef unsigned short SugggestionAlgoType;

// Struct to define a complete suggestion.
struct CompleteSuggestion {
  // The type of the completion: hotel, attraction, etc.
  ::entity::EntityType src_type = ::entity::kEntityTypeInvalid;

  // The source id: hid, city id, etc. This is used to uniquely identify
  // a completion.
  string src_id;

  // The country code for the completion.
  string country = "US";

  // The score for the given completion.
  double base_score = 0;

  double latitude = 0;

  double longitude = 0;

  // The normalized suggestion string. This can be used internally to do
  // various string comparisons.
  string normalized;

  // The display string. This string along with src_id and annotations will
  // be used to map the completion back to r77 understandable data.
  string display;

  // The annotations for the display string.
  vector<string> annotations;

  // The frequency of occurrence of this query in the search logs.
  double freq = 0;

  SERIALIZE(DEFAULT_CUSTOM / src_type*1 / src_id*2 / country*3 / base_score*4 /
            latitude*5 / longitude*6 / normalized*7 / display*8 / freq*9 /
            annotations*10);
};

// The type to use to uniquely define a suggestion.
typedef string SuggestionId;
typedef UNIQUE32(SuggestionId) UniqueSuggestionId;

// Struct used to store different index mappings for algos like
// prefix/bag of words/templates.
struct CompletionIndexItem {
  CompletionIndexItem() = default;
  CompletionIndexItem(const CompletionIndexItem&) = default;
  explicit CompletionIndexItem(const string& id) : suggestion_id(id) {}

  // The suggestion id: This is used to identify the complete suggestion
  // associated with the completion.
  UniqueSuggestionId suggestion_id;

  SERIALIZE(DEFAULT_CUSTOM / suggestion_id*1);
};

// Extended struct used to store different index mappings for algos like
// prefix/bag of words/templates.
struct CompletionIndexItemEx : public CompletionIndexItem {
  using CompletionIndexItem::CompletionIndexItem;
  CompletionIndexItemEx() = default;
  CompletionIndexItemEx(const CompletionIndexItemEx&) = default;
  CompletionIndexItemEx(const CompletionIndexItem& c) :
      CompletionIndexItem(c) {}
  CompletionIndexItemEx(const string& id, double score)
      : CompletionIndexItem(id), index_score(score) {}

  // The score associated with the index. If this is set, this is the value
  // used instead of base_score in the complete_suggestions.
  double index_score = 0;

  SERIALIZE(DEFAULT_CUSTOM / suggestion_id*1 / index_score*2);
};

// Struct used to contain all the run time data associated with a
// complete suggestion.
struct Completion {
  Completion() = default;
  Completion(const Completion&) = default;
  Completion(const CompletionIndexItem& c) : suggestion_id(c.suggestion_id)  {}  // NOLINT
  Completion(const CompletionIndexItemEx& c)  // NOLINT
      : suggestion_id(c.suggestion_id), index_score(c.index_score) {}

  // The suggestion id as copied from CompletionIndexItem.
  SuggestionId suggestion_id;

  // The index score as copied from CompletionIndexItemEx.
  double index_score = 0;

  // The computed score for the completion. This will be modified over time
  // by different online algorithms.
  double score = 0;

  // Whether this is prefix, mid string, synonym, template expansion. etc.
  SugggestionAlgoType algo_type = kCompletionAlgoTypeInvalid;

  // The complete suggestion associated with the completion.
  shared_ptr<CompleteSuggestion> suggestion;

  // The id for the parent if the completion is a child completion.
  SuggestionId parent_id;

  // The debug info associated with each completion.
  string debug_info;

  // This is serialized only for debugging purposes.
  // This should never be returned in production.
  SERIALIZE(DEFAULT_CUSTOM / suggestion_id*1 / index_score*2 / score*3 / algo_type*4 /
            suggestion*5 / parent_id*6 / debug_info*10);
};

// The suggest query that is used as request interface for different suggest
// APIs.
struct SuggestQuery {
  // The input query from the user.
  string input;

  // The currently selected suggestion on which to do further query building.
  SuggestionId selected_id;

  // The user language.
  string user_language;

  // The user country.
  string user_country;

  // The number of suggestions requested.
  // If the number of suggestions is not specified, they are chosen based on
  // the channel.
  int num_suggestions = 0;

  // This will expand to include previous top suggestion/selected suggestion/etc.
  SERIALIZE(DEFAULT_CUSTOM / input*1 / user_language*2 / user_country*3 /
            num_suggestions*4);
};

// The struct used to define the input query to compute suggestions.
struct SuggestRequestInterface : public SuggestQuery {
  SuggestRequestInterface() = default;
  SuggestRequestInterface(const SuggestRequestInterface&) = default;
  explicit SuggestRequestInterface(const SuggestQuery& q) : SuggestQuery(q) {}

  // These fields must be filled before the interface is called.

  // The device channel from which the query was made.
  meta::channel::DeviceChannel device_channel =
      meta::channel::kChannelDesktopWeb;

  // True if the request is from a mobile device. This is there to avoid
  // checking channel every time.
  bool is_mobile = false;

  // Check if debug info is requested.
  bool debug = false;
};

// The struct that is used internally to pass the suggest request around.
// The fields in this struct are modified during different internal computations.
struct SuggestRequest : public SuggestRequestInterface {
  SuggestRequest() = default;
  SuggestRequest(const SuggestRequest&) = default;
  SuggestRequest(const SuggestRequestInterface& r) : SuggestRequestInterface(r) {}  // NOLINT

  // The normalized version of the input query.
  string normalized_query;

  // True if the last word in the query is complete. i.e. last character in the input query was
  // a space.
  bool last_word_complete = false;

  // Alternate versions of the query, possibly from spelling correction, etc.
  // TODO(pramodg): Figure out a way to use it!
  vector<string> alternate_queries;

  // This is for debugging purposes only.
  SERIALIZE(DEFAULT_CUSTOM / input*1 / user_language*2 / user_country*3 /
            num_suggestions*4 / device_channel*5 / is_mobile*6 / debug*7 /
            normalized_query*8 / alternate_queries*9 / last_word_complete*10);
};

// The response for a given SuggestRequest.
struct SuggestResponse {
  // Returns true if the request was successful and the response contains valid
  // data.
  bool success = false;

  // List of comletions in the response.
  vector<Completion> completions;

  // Set to true if the first suggestion is instant search worthy.
  bool enable_instant = false;

  bool Success() const { return success && completions.size(); }

  // This is for debugging purposes only.
  SERIALIZE(success*1 / completions*2);
};

// This is the common context that shared between different algorithms in
// suggest.
struct SuggestContext {
  // If the counter is not null, the counter must be notified as soon as the
  // algo is finished.
  shared_ptr< ::util::threading::Counter> counter;

  // The common thread pool to use.
  ::util::threading::ThreadPool* pool = nullptr;

  // This is the response that has already been computed after merging and
  // sorting the results of the primary algorithms. Some secondary algorithms
  // may choose to use these responses while computing suggestions.
  // NOTE: The contents of this response should not be modified.
  shared_ptr<SuggestResponse> current_response;
};

}  // namespace suggest


#endif  // _META_SUGGEST_COMMON_SUGGEST_DATATYPES_H_
