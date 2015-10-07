// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_COMMON_TWIDDLER_SUGGEST_TWIDDLER_H_
#define _META_SUGGEST_COMMON_TWIDDLER_SUGGEST_TWIDDLER_H_

#include <memory>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/factory/factory.h"
#include "util/factory/factory_extra.h"
#include "util/thread/counters.h"
#include "util/thread/thread_pool.h"

namespace suggest {
namespace twiddle {

// This is the context that an twiddler can use to share different parameters with
// caller and other twiddlers.
struct SuggestTwiddlerContext : public SuggestContext {};

struct SuggestTwiddleRequest {
  SuggestRequest suggest_request;
  shared_ptr<SuggestResponse> suggest_response;
};

struct SuggestTwiddleResponse {
  struct CompletionScore {
    // The score for the given completion.
    double score = 0;
    // Debug info if any.
    string debug_info;
  };

  // Set to true if the twiddler response is valid.
  bool success = false;

  // List of scores associated with the completions.
  // Note: There is a 1-to-1 mapping between the completions in the
  // suggest_response and the scores in completion_scores.
  // Every twiddler that is successful must have scores for each completion.
  vector<CompletionScore> completion_scores;
};

// The interface class for different suggest twiddlers.
// All twiddlers must return a multiplicative factor relative to the
// original score of a completion. This multiplicative factor(x) can be:
// x > 1: The suggestions would be promoted.
// x < 1: The suggestions would be demoted.
// x = 0: The suggestions would be filtered out. // This should be used very
//        very carefully.
class SuggestTwiddler : public Factory<SuggestTwiddler> {
 public:
  virtual ~SuggestTwiddler() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  // The interface function for different suggest twiddlers. The response is filled
  // with relevant scores from the twiddler.
  // If a counter is specified in the context, the algorithm calls Notify()
  // once valid results have been filled into the response.
  // Notes:
  // 1. The response must have 'success' set if the twiddler returns valid data.
  // 2. There must be exactly the same number of scores in the response as the
  //    completions in the request.
  // 2. The response and context object may never be null.
  virtual bool GetScore(const SuggestTwiddleRequest& request,
                        shared_ptr<SuggestTwiddleResponse>& response,
                        shared_ptr<SuggestTwiddlerContext> context) const = 0;

  // Utility function when there is no context specified.
  virtual bool GetScore(const SuggestTwiddleRequest& request,
                       shared_ptr<SuggestTwiddleResponse> response) const {
    return GetScore(request, response,
                    shared_ptr<SuggestTwiddlerContext>(new SuggestTwiddlerContext));
  }
};

}  // namespace twiddle
}  // namespace suggest


#endif  // _META_SUGGEST_COMMON_TWIDDLER_SUGGEST_TWIDDLER_H_
