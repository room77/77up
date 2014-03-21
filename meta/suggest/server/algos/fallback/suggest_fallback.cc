// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/algos/fallback/suggest_fallback.h"

#include "meta/suggest/util/suggest_utils.h"

FLAG_string(suggest_algo_fallback_params, "{}",
            "Default parameters for suggest fallback algo.");

namespace suggest {
namespace algo {

// Initialize the class.
bool SuggestFallback::Initialize() {
  return true;
}

// The interface function for different suggest Algos. The response is filled
// with relevant results from the algorithm. If a counter is specified the
// algorithm calls Notify() once valid results have been filled into the
// response.
int SuggestFallback::GetCompletions(const SuggestRequest& request,
                                    shared_ptr<SuggestResponse> response,
                                    shared_ptr<SuggestAlgoContext> context) const {
  ::util::threading::ScopedCNotify n(context->counter.get());

  // TODO(pramodg): Implement this function.
  response->success = false;
  return 0;
}

// Register bag of words algo.
auto reg_suggest_algo_fallback = SuggestAlgo::bind(
    "suggest_algo_fallback", gFlag_suggest_algo_fallback_params,
    InitializeConfigureConstructor<SuggestFallback, string>());

}  // namespace algo
}  // namespace suggest
