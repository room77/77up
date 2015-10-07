// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/algos/attributes/suggest_attribute.h"

#include "base/logging.h"
#include "util/string/strutil.h"
#include "util/entity/entity_id.h"
#include "util/entity/entity_type.h"

FLAG_string(suggest_algo_attribute_default_key, "m/default_order",
            "Default parameters for suggest bag of words algo.");

namespace suggest {
namespace algo {

// Initialize the class.
bool SuggestAttribute::Initialize() {
  LOG(INFO) << "Initializing Attribute using params: " << params().ToJSON();

  if (params().attribute_index_algo_name.empty()) {
    LOG(ERROR) << "No attribute index algo specified!";
    return false;
  }

  // Init the word suggest algo that is used to fetch prefixes per word.
  attribute_index_algo_ = make_shared(params().attribute_index_algo_name);

  if (attribute_index_algo_ == nullptr) {
    LOG(ERROR) << "Invalid algo: " << params().attribute_index_algo_name;
    return false;
  }

  // Initialize the default response.
  InitDefaultAttributes();

  return true;
}

bool SuggestAttribute::InitDefaultAttributes() {
  SuggestRequest req;
  req.normalized_query = gFlag_suggest_algo_attribute_default_key;
  default_attributes_.reset(new SuggestResponse);
  attribute_index_algo_->GetCompletions(req, default_attributes_);

  return default_attributes_->success;
}

// The interface function for different suggest Algos. The response is filled
// with relevant results from the algorithm. If a counter is specified the
// algorithm calls Notify() once valid results have been filled into the
// response.
int SuggestAttribute::GetCompletions(const SuggestRequest& request,
    shared_ptr<SuggestResponse> response,
    shared_ptr<SuggestAlgoContext> context) const {
  ::util::threading::ScopedCNotify n(context->counter.get());

  VLOG(3) << "Attribute: " << params().id << ": [" << request.normalized_query << "]";

  if (context->current_response == nullptr ||
      !context->current_response->success) {
    LOG(ERROR) << "Attribute algo called without setting primary response.";
    response->success = false;
    return 0;
  }

  // Get the attributes for the top params.max_attribute_candidates parent completions.
  for (int i = 0; i < context->current_response->completions.size() &&
      i < params().max_attribute_candidates; ++i) {
    const Completion& completion = context->current_response->completions[i];
    shared_ptr<SuggestResponse> attr_resp(new SuggestResponse);
    GetAttributes(request, completion, attr_resp);
    if (!attr_resp->Success()) continue;

    response->completions.reserve(response->completions.size() +
                                  attr_resp->completions.size());
    response->completions.insert(response->completions.end(),
                                 attr_resp->completions.begin(),
                                 attr_resp->completions.end());
  }

  VLOG(4) << "Attribute: " << params().id << ": Found "
         << response->completions.size() << " suggestions for ["
         << request.normalized_query << "]";

  response->success = true;
  return response->completions.size();
}

int SuggestAttribute::GetAttributes(const SuggestRequest& request,
    const Completion& parent, shared_ptr<SuggestResponse> response) const {
  // Prepare the attribute request.
  SuggestRequest attr_req = request;
  attr_req.normalized_query = parent.suggestion_id;

  // Fetch the attributes for the given request from the index algo.
  attribute_index_algo_->GetCompletions(attr_req, response);

  // Do not allow hotels to have default expansions.
  // We may want to make this more restrictive in the future.
  if (!response->Success() &&
      parent.suggestion->src_type != entity::kEntityTypeHotel) {
    // We assume the score is already valid and
    if (default_attributes_->Success()) {
      response->completions = default_attributes_->completions;
      response->success = true;
      for (Completion& completion : response->completions)
        completion.debug_info.append("| ATTRIBUTE: default list");
    }
  }

  if (!response->Success()) {
    VLOG(5) << "No attribute for completion: " << parent.suggestion_id;
    return 0;
  }

  // Now we have the attributes for a parent. Set the parent id.
  for (Completion& completion : response->completions) {
    completion.parent_id = parent.suggestion_id;

    PrepareChildCompletionFromParent(parent, &completion);
  }

  return response->completions.size();
}

void SuggestAttribute::PrepareChildCompletionFromParent(const Completion& parent,
                                                        Completion* child) const {
  // Fix the score. We assume the attribute score is the % global engagement
  // with a particular filter. i.e. how may times a is a particular filter
  // choses divided by the number of times any filter is chosen.
  // Thus, multiplying this by the parent score should give us a fair
  // approximation on the attribute score.
  child->score *= parent.score;

  // We need to ensure that the suggestion id is always unique.
  child->suggestion_id = strutil::JoinTwoStrings(parent.suggestion_id, child->suggestion_id,
                                                 ::entity::GetMultipleEntitiesSeparator());
}

}  // namespace algo
}  // namespace suggest
