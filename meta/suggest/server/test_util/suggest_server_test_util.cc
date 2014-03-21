// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/suggest/server/test_util/suggest_server_test_util.h"

#include "meta/suggest/common/suggest_datatypes.h"

namespace suggest {
namespace test {

SuggestRequest MakeMockSuggestRequest(const string& norm_query) {
  SuggestRequest req;

  req.normalized_query = norm_query;

  req.input = norm_query;
  req.user_country = "US";
  req.user_language = "en";
  req.num_suggestions = 5;

  return req;
}

CompleteSuggestion MakeMockCompleteSuggestion(double score,
                                              const string& norm) {
  CompleteSuggestion suggestion;
  suggestion.base_score = score;
  suggestion.freq = score / 2;
  suggestion.country = "US";
  suggestion.display = norm;
  suggestion.normalized = norm;

  suggestion.src_id = "c/" + norm;
  suggestion.src_type = entity::kEntityTypeCity;
  suggestion.annotations = {"CA", "US"};
  return suggestion;
}

Completion MakeMockCompletion(double score, const string& norm) {
  Completion completion;
  completion.suggestion.reset(
      new CompleteSuggestion(MakeMockCompleteSuggestion(score, norm)));
  completion.score = completion.suggestion->base_score;
  completion.algo_type = kCompletionAlgoTypePrefix;
  completion.suggestion_id = completion.suggestion->src_id;

  return completion;
}

SuggestResponse MakeMockSuggestResponse(const string& id, double base_score,
                                        int n) {
  SuggestResponse response;
  response.completions.reserve(n);
  for (int i = 0; i < n; ++i) {
    response.completions.push_back(
        MakeMockCompletion(100 * (n - i), id + std::to_string(i)));
  }
  response.success = true;
  return response;
}

}  // namespace test
}  // namespace suggest
