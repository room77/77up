// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_SUGGESTIONS_SUGGESTIONS_H_
#define _META_SUGGEST_SERVER_SUGGESTIONS_SUGGESTIONS_H_

#include <memory>

#include "base/common.h"

#include "meta/suggest/common/suggest_datatypes.h"

namespace suggest {

// Forward declare the manager.
class SuggestionManager;

// Class for serving suggestions for a given request.
// This class merges suggestions from different algorithms and serves them
// after rescoring/re-ranking all suggestions.
// NOTE: This class is not thread safe and we expect one object of the class
// to be created per suggest request.
class Suggestions {
 public:
  // The suggestions requires a suggestionmanager for different stateful
  // objects. Make sure this is a valid value.
  // The caller still has ownership of the manager and needs to ensure that
  // the manager lasts the lifetime of the Suggestions class.
  explicit Suggestions(SuggestionManager* manager) : manager_(manager) {}

  virtual ~Suggestions() {}

  // Returns the completions for a given suggest request.
  int GetCompletions(const SuggestRequestInterface& request,
                     shared_ptr<SuggestResponse> response) {
    if (!PrepareRequest(request)) return 0;
    response_ = response;
    return GetCompletions();
  }

  SuggestionManager* manager() { return manager_; }

 protected:
  // Initializes request_ based on the input request.
  // Returns true if the request could be prepared correctly and false otherwise.
  virtual bool PrepareRequest(const SuggestRequestInterface& req);

  // Main workhorse to generate and return completions.
  virtual int GetCompletions();

  // Runs the primary flow.
  // Returns true if it fould at least one completion for the query.
  bool RunPrimaryFlow();

  // Runs the fallback flow.
  bool RunFallbackFlow();

  // Runs the secondary flow.
  bool RunSecondaryFlow();

  // Computes the primary suggestions for the query.
  int GetPrimarySuggestions();

  // Computes the fallback suggestions for the query.
  int GetFallbackSuggestions();

  // Computes the primary suggestions for the query.
  int GetSecondarySuggestions();

  // Twiddles the primary suggestions in the response.
  bool TwiddlePrimaryResponse();

  // Twiddles the secondary suggestions in the response.
  bool TwiddleSecondaryResponse();

  // Dedups the suggestions in the response.
  void DedupResponse();

  // Merge primary and secondary response.
  void MergePrimaryAndSecondaryResponse();

  // Finalize the Suggestions.
  void Finalize();

  // Finalize the Suggestions.
  void FixPositions();

  // Check if the top suggestion is instant worthy.
  void CheckTopCompletionInstantWorthy();

  SuggestionManager* manager_;

  // The suggest request.
  SuggestRequest request_;

  // The suggest response for the request.
  shared_ptr<SuggestResponse> response_;

  // The intermediate secondary response for the request.
  shared_ptr<SuggestResponse> secondary_response_;
};

}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_SUGGESTIONS_SUGGESTIONS_H_
