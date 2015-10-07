// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/suggestions/suggestions.h"

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <utility>

#include "meta/suggest/common/dedup/suggest_dedup.h"
#include "meta/suggest/common/rankers/suggest_comparator.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/common/twiddler/suggest_twiddler.h"
#include "meta/suggest/common/twiddler/util/suggest_twiddler_utils.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "meta/suggest/server/suggestions/suggestion_manager.h"
#include "meta/suggest/util/suggest_utils.h"
#include "util/region_data/utils.h"
#include "util/templates/container_util.h"
#include "util/thread/thread_pool.h"
#include "util/time/simple_timer.h"

FLAG_string(suggest_input_default_country, "US",
            "The default input country to use if none specified.");

FLAG_string(suggest_input_default_language, "en",
            "The default input language to use if none specified.");

FLAG_int(suggest_input_default_web_suggestions, 10,
         "The default number of suggestions for web.");

FLAG_int(suggest_input_default_mobile_suggestions, 5,
         "The default number of suggestions for mobile.");

FLAG_int(suggest_min_secondary_suggestions, 6,
         "The min number of secondary suggestions to keep for evaluation.");

FLAG_int(suggest_max_suggestions_multiplier, 6,
         "The max number of suggestions to keep for evaluation. We drop "
         "suggestions that ranked lower than request.num_suggestions * "
         "multiplier.");

FLAG_int(suggest_top_suggestion_min_freq_for_instant, 10,
         "This is the minimum number of times the top suggestion must have "
         "already been searched for it to be considered instant worthy.");

FLAG_double(suggest_top_suggestion_min_selection_probability_for_instant, 0.4,
            "This is the minimum probability of selection the top suggestion "
            "must have among visible suggestions for it to be considered "
            "instant worthy.");

namespace suggest {

namespace {

inline void SortResponse(SuggestResponse* response, int max_suggestions) {
  // Sort the responses upto max suggestions.
  partial_sort(response->completions.begin(),
     response->completions.begin() +
         min<int>(max_suggestions, response->completions.size()),
     response->completions.end(), rank::better_completion());
}

inline void TrimResponse(SuggestResponse* response, int max_suggestions) {
  if (response->completions.size() > max_suggestions)
    response->completions.erase(
        response->completions.begin() + max_suggestions,
        response->completions.end());
}

inline void SortAndTrimResponse(SuggestResponse* response, int max_suggestions) {
  if (response->completions.empty()) return;
  SortResponse(response, max_suggestions);
  TrimResponse(response, max_suggestions);
}

}  // namespace


bool Suggestions::PrepareRequest(const SuggestRequestInterface& req) {
  // Get the default values.
  request_ = req;

  // Normalize the user query.
  request_.normalized_query = region_data::utils::NormalizeString(
      req.input);
  if (request_.normalized_query.empty()) return false;

  // If the last character in the input query is a space, add it back.
  if (request_.input[request_.input.size() - 1] == ' ') {
    request_.normalized_query += " ";
    request_.last_word_complete = true;
  }

  // Fix language and country.
  if (request_.user_country.empty())
    request_.user_country = gFlag_suggest_input_default_country;

  if (request_.user_language.empty())
    request_.user_language = gFlag_suggest_input_default_language;

  // Fix the number of suggetions.
  if (request_.num_suggestions <= 0) {
    request_.num_suggestions = request_.is_mobile ?
        gFlag_suggest_input_default_mobile_suggestions :
        gFlag_suggest_input_default_web_suggestions;
  }

  return true;
}

int Suggestions::GetCompletions() {
  ::util::time::ScopedMilliSecondsTimer timer(
      "Total [" + request_.normalized_query + "] (ms):", 3);

  VLOG(3) << "Get completions for [" << request_.normalized_query << "]";
  // Run the primary flow to fetch suggestions for the query.
  // If we cannot find any suggestions, run the fallback flow.
  if (!RunPrimaryFlow())
    RunFallbackFlow();

  // After we have some suggestions, run the secondary flow.
  RunSecondaryFlow();

  // After we have run all possible algorithms to fetch suggestions, finalize
  // the suggestions.
  Finalize();

  VLOG(3) << "Found " << response_->completions.size() << " completions for ["
         << request_.normalized_query << "]";

  return response_->Success() ? response_->completions.size() : 0;
}

bool Suggestions::RunPrimaryFlow() {
  ::util::time::ScopedMilliSecondsTimer timer(
      "Primary [" + request_.normalized_query + "] (ms):", 4);

  // We may want to run other things in parallel to computing the primary
  // suggestions in future (e.g. spell corrections, etc.).
  // For those, use counters to run them in parallel. However, the primary
  // suggestions should still always be fetched in the main thread.

  // Get the suggestions.
  if (!GetPrimarySuggestions()) return false;

  // Rerank the suggestions.
  TwiddlePrimaryResponse();

  // Sort the suggestions.
  SortAndTrimResponse(response_.get(),
                      request_.num_suggestions * gFlag_suggest_max_suggestions_multiplier);

  // Dedup the suggestions.
  DedupResponse();

  TrimResponse(response_.get(), request_.num_suggestions);

  // If we have at least one suggetion after everything, we mark the primary
  // flow as success.
  return response_->Success() ? response_->completions.size() : 0;
}

bool Suggestions::RunFallbackFlow() {
  // TODO(pramodg): Implement this flow.
  // This will use spell correction, alternate names, etc. to fill in the
  // suggestions.
  return false;
}

bool Suggestions::RunSecondaryFlow() {
  ::util::time::ScopedMilliSecondsTimer timer(
      "Secondary [" + request_.normalized_query + "] (ms):", 4);

  // If there is no valid response, return.
  if (!response_->Success()) return false;

  // Get the suggestions.
  if (!GetSecondarySuggestions()) return false;

  // Rerank the suggestions.
  TwiddleSecondaryResponse();

  int num_secondary_suggestions = max<int>(request_.num_suggestions - response_->completions.size(),
      gFlag_suggest_min_secondary_suggestions);

  // TODO(pramodg): Here we should not trim to final size once we have a secondary deduper.
  SortAndTrimResponse(secondary_response_.get(), num_secondary_suggestions);

  // Merge primary and secondary suggestions.
  MergePrimaryAndSecondaryResponse();

  // Sort the suggestions.
  SortAndTrimResponse(response_.get(),
                      request_.num_suggestions * gFlag_suggest_max_suggestions_multiplier);

  // Dedup the suggestions.
  DedupResponse();

  TrimResponse(response_.get(), request_.num_suggestions);

  // If we have at least one suggetion after everything, we mark the secondary
  // flow as success.
  return response_->Success() ? response_->completions.size() : 0;
}

int Suggestions::GetPrimarySuggestions() {
  using algo::SuggestAlgoContext;

  // Setup the context for the algos.
  shared_ptr<SuggestAlgoContext> context(new SuggestAlgoContext);
  context->pool = manager_->thread_pool();

  manager()->primary_algo()->GetCompletions(request_, response_, context);
  return response_->Success();
}

int Suggestions::GetFallbackSuggestions() {
  using algo::SuggestAlgoContext;

  // Setup the context for the algos.
  shared_ptr<SuggestAlgoContext> context(new SuggestAlgoContext);
  context->pool = manager_->thread_pool();

  manager()->fallback_algo()->GetCompletions(request_, response_, context);
  return response_->Success();
}

int Suggestions::GetSecondarySuggestions() {
  using algo::SuggestAlgoContext;

  // Setup the context for the algos.
  shared_ptr<SuggestAlgoContext> context(new SuggestAlgoContext);
  context->pool = manager_->thread_pool();
  context->current_response = response_;

  secondary_response_.reset(new SuggestResponse);

  manager()->secondary_algo()->GetCompletions(request_, secondary_response_,
                                              context);

  if (!secondary_response_->Success()) return 0;

  return secondary_response_->completions.size();
}

bool Suggestions::TwiddlePrimaryResponse() {
  using twiddle::SuggestTwiddleRequest;
  using twiddle::SuggestTwiddleResponse;
  using twiddle::SuggestTwiddlerContext;

  SuggestTwiddleRequest twiddler_request = { request_, response_ };
  shared_ptr<SuggestTwiddleResponse> twiddler_response(
      new SuggestTwiddleResponse);

  shared_ptr<SuggestTwiddlerContext> twiddler_context(
      new SuggestTwiddlerContext);
  twiddler_context->pool = manager_->thread_pool();

  if (!manager()->primary_twiddler()->GetScore(twiddler_request, twiddler_response,
                                               twiddler_context)) {
    LOG(WARNING) << "[" << request_.normalized_query
            << "]: Failed to twiddle primary suggestions";
    return false;
  }

  // Update the suggest response with the twiddler data.
  bool status = UpdateCompletionsWithTwiddlerResponse(request_, *twiddler_response,
                                                      response_.get());

  return status;
}

bool Suggestions::TwiddleSecondaryResponse() {
  using twiddle::SuggestTwiddleRequest;
  using twiddle::SuggestTwiddleResponse;
  using twiddle::SuggestTwiddlerContext;

  SuggestTwiddleRequest twiddler_request = { request_, secondary_response_ };
  shared_ptr<SuggestTwiddleResponse> twiddler_response(
      new SuggestTwiddleResponse);

  shared_ptr<SuggestTwiddlerContext> twiddler_context(
      new SuggestTwiddlerContext);
  twiddler_context->pool = manager_->thread_pool();
  twiddler_context->current_response = response_;

  if (!manager()->secondary_twiddler()->GetScore(twiddler_request, twiddler_response,
                                                 twiddler_context)) {
    LOG(WARNING) << "[" << request_.normalized_query
            << "]: Failed to twiddle secondary suggestions";
    return false;
  }

  // Update the secondary response with the twiddler data.
  bool status = UpdateCompletionsWithTwiddlerResponse(request_, *twiddler_response,
                                                      secondary_response_.get());
  return status;
}

void Suggestions::MergePrimaryAndSecondaryResponse() {
  if (!secondary_response_->success || secondary_response_->completions.empty())
    return;

  // Insert the suggestions from the secondary algorithms to the response.
  response_->completions.insert(response_->completions.end(),
                                secondary_response_->completions.begin(),
                                secondary_response_->completions.end());

  // Mark the response as success.
  response_->success = true;
}

void Suggestions::DedupResponse() {
  using dedup::SuggestDedup;
  for (dedup::SuggestDedup::shared_proxy deduper : manager()->dedupers()) {
    if (response_->completions.empty()) break;

    int deduped = deduper->Dedup(request_, response_);
    VLOG(4) << "[" << request_.normalized_query << "]: Deduped " << deduped
           << " suggestions.";
  }
}

void Suggestions::Finalize() {
  if (!response_->Success()) return;

  TrimResponse(response_.get(), request_.num_suggestions);

  // Fix the positions for the completions.
  FixPositions();

  CheckTopCompletionInstantWorthy();

  for (Completion& completion : response_->completions) {
    completion.debug_info.append(" | src: " +
                                 util::GetAlgoNameFromType(completion.algo_type));
  }
}

void Suggestions::FixPositions() {
  // We need to fix the positions of the completions here.
  // We rearrange suggestions in a way that all the children for a given parent
  // are right next to it.

  // Map of parent_suggestion_id -> <original index, child completion>
  unordered_map<SuggestionId, vector<pair<int,
      std::reference_wrapper<Completion>>>> parent_child_map;

  for (int i = 0; i < response_->completions.size(); ++i) {
    Completion& completion = response_->completions[i];
    if (completion.parent_id.empty()) continue;

    // Associate the child with its parent.
    parent_child_map[completion.parent_id].push_back(
        make_pair(i, std::ref(completion)));
  }

  vector<Completion> reordered_completions;
  reordered_completions.reserve(response_->completions.size());
  for (Completion& completion : response_->completions) {
    if (!completion.parent_id.empty()) continue;

    // Add each parent to the reordered list.
    reordered_completions.push_back(completion);

    // Add all its children right after it.
    static const vector<pair<int, std::reference_wrapper<Completion>>> kDefault;
    const auto& children = ::util::tl::FindWithDefault(parent_child_map,
        completion.suggestion_id, kDefault);
    VLOG(5) << completion.suggestion_id << ": No. of children : "
            << children.size();
    if (children.empty()) continue;
    for (const auto& p : children) {
      reordered_completions.push_back(p.second);

      if (p.first != reordered_completions.size())
        p.second.get().debug_info.append(" | Moved child from " +
            to_string(p.first) + " to " +
            to_string(reordered_completions.size()));
    }
  }
  response_->completions = reordered_completions;
}

void Suggestions::CheckTopCompletionInstantWorthy() {
  // Check it the top suggestion has enough frequency to be considered.
  // Note: We already know we have at least 1 completion. No need to check for
  // size.
  Completion& top_completion = response_->completions[0];
  if (top_completion.suggestion->freq <
      gFlag_suggest_top_suggestion_min_freq_for_instant) return;

  double total_score = 0;
  for (const Completion& completion : response_->completions) {
    if (!completion.parent_id.empty()) continue;
    total_score += completion.score;
  }

  if (top_completion.score < total_score *
      gFlag_suggest_top_suggestion_min_selection_probability_for_instant)
    return;

  // All checks passed. Lets turn instant on.
  response_->enable_instant = true;
}

}  // namespace suggest
