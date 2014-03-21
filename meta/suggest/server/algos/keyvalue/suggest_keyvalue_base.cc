// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue_base.h"

#include "meta/suggest/util/suggest_utils.h"

namespace suggest {
namespace algo {

// Initialize the class.
bool SuggestKeyValueBase::Initialize() {
  params_.name = util::GetAlgoNameFromType(params().type);
  ASSERT_NE("invalid algo", params().name);

  LOG(INFO) << "Initializing " << params().name << " with params: "
         << params().ToJSON();

  falcon_ = falcon::SuggestFalcon::make_shared(params().falcon);
  if (falcon_ == nullptr) {
    LOG(ERROR) << "Invalid Falcon id!" << params().falcon;
    return false;
  }

  if (params().file.empty()) {
    LOG(ERROR) << "No file specified!";
    return false;
  }

  return true;
}

// The interface function for different suggest Algos. The response is filled
// with relevant results from the algorithm. If a counter is specified the
// algorithm calls Notify() once valid results have been filled into the
// response.
int SuggestKeyValueBase::GetCompletions(const SuggestRequest& request,
    shared_ptr<SuggestResponse> response,
    shared_ptr<SuggestAlgoContext> context) const {
  ::util::threading::ScopedCNotify n(context->counter.get());

  int num_completions = FindCompletions(request, response, context);

  if (!num_completions) {
    VLOG(3) << params().name << ": Found no suggestions for ["
           << request.normalized_query << "]";
    return 0;
  }

  // Get the complete suggestions corresponding to the completions.
  falcon_->AddCompleteSuggestions(response);

  response->success = true;
  VLOG(4) << params().name << ": Found " << num_completions
         << " suggestions for [" << request.normalized_query << "]";

  return num_completions;
}

}  // namespace algo
}  // namespace suggest
