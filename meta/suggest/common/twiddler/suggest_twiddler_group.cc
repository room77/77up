// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/common/twiddler/suggest_twiddler_group.h"

#include <algorithm>
#include <functional>

#include "util/time/simple_timer.h"

namespace suggest {
namespace twiddle {

// Initialize the class.
bool SuggestTwiddlerGroup::Initialize() {
  LOG(INFO) << "Initializing Twiddler group: " << params().id << " with params: "
         << params().ToJSON();

  if (params().twiddler_params.empty()) {
    LOG(INFO) << "No twiddlers specified!";
    return false;
  }

  // We have a one to one mapping between params and twiddlers.
  suggest_twiddlers_.reserve(params().twiddler_params.size());
  for (SuggestTwiddlerGroupParams::TwiddlerParams& twiddler_param :
      params_.twiddler_params) {
    shared_proxy suggest_twiddler = make_shared(twiddler_param.id);
    ASSERT_NOTNULL(suggest_twiddler) << "Could not init twiddler: " << twiddler_param.ToJSON();
    suggest_twiddlers_.push_back(suggest_twiddler);

    twiddler_param.operator_ = ::meta::rank::Operator::make_shared(twiddler_param.op);
    ASSERT_NOTNULL(twiddler_param.operator_)
        << "Could not init operator for twiddler: " << twiddler_param.ToJSON();

    if (twiddler_param.required) ++params_.num_required_twiddlers_;
    else ++params_.num_optional_twiddlers_;
  }
  return true;
}

bool SuggestTwiddlerGroup::GetScore(const SuggestTwiddleRequest& request,
                                    shared_ptr<SuggestTwiddleResponse>& response,
                                    shared_ptr<SuggestTwiddlerContext> context) const {
  ::util::threading::ScopedCNotify n(context->counter.get());
  ::util::time::ScopedMilliSecondsTimer timer(
      params().id + ": [" + request.suggest_request.normalized_query + "] (ms):",
      3);

  VLOG(3) << params().id << ": [" << request.suggest_request.normalized_query << "]";
  if (!request.suggest_response->success ||
      request.suggest_response->completions.empty()) {
    LOG(INFO) << "Will not twiddle invalid response.";
    return false;
  }

  // Prepare the contexts for the different twiddlers.
  shared_ptr<SuggestTwiddlerContext> required_twiddlers_context(
      new SuggestTwiddlerContext(*context));

  shared_ptr<SuggestTwiddlerContext> optional_twiddlers_context(
      new SuggestTwiddlerContext(*context));

  required_twiddlers_context->counter.reset(
      new ::util::threading::Counter(params().num_required_twiddlers_));
  optional_twiddlers_context->counter.reset(
      new ::util::threading::Counter(params().num_optional_twiddlers_));

  vector<shared_ptr<SuggestTwiddleResponse>> twiddler_responses(suggest_twiddlers_.size());
  for (int i = 0; i < suggest_twiddlers_.size(); ++i) {
    // Prepare the request and the response.
    shared_ptr<SuggestTwiddleResponse>& twiddler_response = twiddler_responses[i];
    twiddler_response = shared_ptr<SuggestTwiddleResponse>(new SuggestTwiddleResponse);

    shared_ptr<SuggestTwiddlerContext>& twiddler_context =
        params().twiddler_params[i].required ?
            required_twiddlers_context : optional_twiddlers_context;

    const shared_proxy& twiddler = suggest_twiddlers_[i];
    auto func = [=]() mutable {
          twiddler->GetScore(request, twiddler_response, twiddler_context);
        };

    if (context->pool != nullptr) context->pool->Add(func);
    else func();
  }

  // Wait for all the required twiddlers to finish.
  required_twiddlers_context->counter->WaitWithTimeout(
      params().timeout_required_twiddlers_ms);

  // Wait for the optional algorithms to finish.
  optional_twiddlers_context->counter->WaitWithTimeout(
      params().timeout_optional_twiddlers_ms);

  // Combine the scores from all twiddlers.
  for (int i = 0; i < suggest_twiddlers_.size(); ++i) {
    const SuggestTwiddlerGroupParams::TwiddlerParams& twiddler_param =
        params().twiddler_params[i];

    shared_ptr<SuggestTwiddleResponse>& twiddler_response = twiddler_responses[i];
    // Verify the twiddler response.
    if (!twiddler_response->success ||
        twiddler_response->completion_scores.empty() ||
        twiddler_response->completion_scores.size() !=
            request.suggest_response->completions.size()) {
      LOG(INFO) << twiddler_param.id << ": ["
             << request.suggest_request.normalized_query << "] failed." ;
      continue;
    }

    CombineTwiddlerScore(twiddler_param, twiddler_response, response.get());
  }

  response->success = (response->completion_scores.size() ==
      request.suggest_response->completions.size());
  return response->success;
}

void SuggestTwiddlerGroup::CombineTwiddlerScore(
    const SuggestTwiddlerGroupParams::TwiddlerParams& twiddler_param,
    shared_ptr<SuggestTwiddleResponse>& twiddler_response,
    SuggestTwiddleResponse* combined_response) const {

  // Multiply the twiddler score by its merge weight.
  for(SuggestTwiddleResponse::CompletionScore& completion_score :
      twiddler_response->completion_scores)
    completion_score.score *= twiddler_param.weight;

  // If this is the first response, set the combined response to the
  // twiddler response.
  bool assign = false;
  if (combined_response->completion_scores.empty()) {
    combined_response->completion_scores.resize(
        twiddler_response->completion_scores.size());
    assign = true;
  }

  // Get the operator function.
  auto func = twiddler_param.operator_->operator ()();

  // Go over each completion score and merge it with the combined score.
  for (int i = 0; i < twiddler_response->completion_scores.size(); ++i) {
    SuggestTwiddleResponse::CompletionScore& combined_score =
        combined_response->completion_scores[i];
    SuggestTwiddleResponse::CompletionScore& twiddler_score =
        twiddler_response->completion_scores[i];

    combined_score.debug_info.append(" # " + twiddler_param.id + " : " +
                                     std::to_string(twiddler_score.score) +
                                     " (" + twiddler_score.debug_info + ")");

    if (assign)
      combined_score.score = twiddler_score.score;
    else
      combined_score.score = func(combined_score.score, twiddler_score.score);
  }
}

}  // namespace twiddle
}  // namespace suggest
