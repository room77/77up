// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/algos/suggest_algo_group.h"

#include <algorithm>
#include <functional>

#include "util/time/simple_timer.h"

namespace suggest {
namespace algo {

// Initialize the class.
bool SuggestAlgoGroup::Initialize() {
  LOG(INFO) << "Initializing Algo group: " << params().id << " with params: "
         << params().ToJSON();

  if (params().algo_params.empty()) {
    LOG(INFO) << "No algos specified!";
    return false;
  }

  // We have a one to one mapping between params and algos.
  suggest_algos_.reserve(params().algo_params.size());
  for (SuggestAlgoGroupParams::AlgoParams& algo_param : params_.algo_params) {
    shared_proxy suggest_algo = make_shared(algo_param.id);
    ASSERT_NOTNULL(suggest_algo) << "Could not init algo: " << algo_param.ToJSON();
    suggest_algos_.push_back(suggest_algo);

    algo_param.merger_ = merge::CompletionsMerger::make_shared(algo_param.op);
    ASSERT_NOTNULL(algo_param.merger_)
        << "Could not init merger for algo: " << algo_param.ToJSON();

    if (algo_param.required) ++params_.num_required_algos_;
    else ++params_.num_optional_algos_;
  }
  return true;
}

// The interface function for different suggest Algos. The response is filled
// with relevant results from the algorithm. If a counter is specified the
// algorithm calls Notify() once valid results have been filled into the
// response.
int SuggestAlgoGroup::GetCompletions(const SuggestRequest& request,
                                     shared_ptr<SuggestResponse> response,
                                     shared_ptr<SuggestAlgoContext> context) const {
  ::util::threading::ScopedCNotify n(context->counter.get());
  ::util::time::ScopedMilliSecondsTimer timer(
      params().id + ": [" + request.normalized_query + "] (ms):", 3);

  VLOG(3) << params().id << ": [" << request.normalized_query << "]";

  // Prepare the contexts for the different algorithms.
  shared_ptr<SuggestAlgoContext> required_algos_context(
      new SuggestAlgoContext(*context));

  shared_ptr<SuggestAlgoContext> optional_algos_context(
      new SuggestAlgoContext(*context));

  required_algos_context->counter.reset(
      new ::util::threading::Counter(params().num_required_algos_));
  optional_algos_context->counter.reset(
      new ::util::threading::Counter(params().num_optional_algos_));

  vector<shared_ptr<SuggestResponse>> algo_responses(suggest_algos_.size());
  for (int i = 0; i < suggest_algos_.size(); ++i) {
    // Prepare the request and the response.
    shared_ptr<SuggestResponse>& algo_response = algo_responses[i];
    algo_response = shared_ptr<SuggestResponse>(new SuggestResponse);

    shared_ptr<SuggestAlgoContext>& algo_context = params().algo_params[i].required ?
        required_algos_context : optional_algos_context;

    const shared_proxy& algo = suggest_algos_[i];
    auto func = [=]() mutable {
          algo->GetCompletions(request, algo_response, algo_context);
        };

    if (context->pool != nullptr) context->pool->Add(func);
    else func();
  }

  // Wait for all the required algorithms to finish.
  required_algos_context->counter->WaitWithTimeout(
      params().timeout_required_algos_ms);

  unordered_map<SuggestionId, Completion> combined_suggestions;
  // Add all the results from finished *required* algos.
  for (int i = 0; i < suggest_algos_.size(); ++i) {
    const SuggestAlgoGroupParams::AlgoParams& algo_param = params().algo_params[i];
    if (!algo_param.required) continue;
    MergeSuggestionsFromAlgo(request, algo_param, algo_responses[i],
                             &combined_suggestions);
  }

  // If the total suggestions are less than the required number, allow more time for optional
  // algorithms to finish.
  if (combined_suggestions.size() < request.num_suggestions) {
    optional_algos_context->counter->WaitWithTimeout(
        params().timeout_optional_algos_ms);
  }

  // Add all the results from finished *optional* algos.
  for (int i = 0; i < suggest_algos_.size(); ++i) {
    const SuggestAlgoGroupParams::AlgoParams& algo_param = params().algo_params[i];

    if (algo_param.required) continue;

    MergeSuggestionsFromAlgo(request, algo_param, algo_responses[i],
                             &combined_suggestions);
  }

  // Fill in the suggestions.
  response->completions.reserve(response->completions.size() +
                                combined_suggestions.size());
  for (const auto& p : combined_suggestions)
    response->completions.push_back(p.second);

  VLOG(3) << params().id << " found " << combined_suggestions.size()
          << " suggestions for query : [" << request.normalized_query << "]";

  response->success = true;
  return response->completions.size();
}

void SuggestAlgoGroup::MergeSuggestionsFromAlgo(const SuggestRequest& request,
    const SuggestAlgoGroupParams::AlgoParams& algo_param,
    shared_ptr<SuggestResponse>& algo_response,
    unordered_map<SuggestionId, Completion>* combined_suggestions) const {
  if (!algo_response->success) {
    VLOG(3) << algo_param.id << " failed for query ["
           << request.normalized_query << "]";
    return;
  }

  VLOG(4) << algo_param.id << " returned " << algo_response->completions.size()
         << " completions for query [" << request.normalized_query << "]";

  if (algo_response->completions.empty()) {
    VLOG(3) << algo_param.id << " returned no completions for query ["
           << request.normalized_query << "]";
    return;
  }

  for (Completion& completion : algo_response->completions) {
    // Multiply the score from an algo with its weight.
    completion.score *= algo_param.weight;
    auto iter = combined_suggestions->find(completion.suggestion_id);
    if (iter == combined_suggestions->end()) {
      // Add the new suggestion.
      (*combined_suggestions)[completion.suggestion_id] = completion;
      continue;
    }

    // We have already seen this suggestion before, lets merge.
    algo_param.merger_->Merge(&(iter->second), completion);
  }
}

}  // namespace algo
}  // namespace suggest
