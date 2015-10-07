// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/algos/bag_of_words/suggest_bag_of_words.h"

#include <algorithm>
#include <functional>
#include <unordered_set>

#include "meta/suggest/common/rankers/suggest_comparator.h"
#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue.h"
#include "meta/suggest/server/algos/util/suggest_algo_utils.h"

#include "util/templates/comparator.h"

FLAG_string(suggest_algo_bow_params, "{"
              "\"word_suggest_algo_name\": \"suggest_algo_bow_index\","
              "\"max_suggestions_multiplier\": 4,"
              "\"max_boost\": 5,"
            "}",
            "Default parameters for suggest bag of words algo.");

FLAG_string(suggest_algo_bow_index_params, "{"
              "\"type\": " +
                  std::to_string(suggest::kCompletionAlgoTypeBagOfWords) + ","
              "\"falcon\": \"suggest_falcon_primary\","
              "\"file\": \"/home/share/data/suggest/locations/index/current/bow/bow_index.dat\","
            "}",
            "Default parameters for suggest bow kv index algorithm.");

namespace suggest {
namespace algo {

// Initialize the class.
bool SuggestBagOfWords::Initialize() {
  LOG(INFO) << "Initializing BOW using params: " << params().ToJSON();

  if (params().word_suggest_algo_name.empty()) {
    LOG(INFO) << "No algo specified!";
    return false;
  }

  // Init the word suggest algo that is used to fetch prefixes per word.
  word_suggest_algo_ = make_shared(params().word_suggest_algo_name);
  ASSERT_NOTNULL(word_suggest_algo_) << "Invalid algo: "
                                     << params().word_suggest_algo_name;
  return true;
}

// The interface function for different suggest Algos. The response is filled
// with relevant results from the algorithm. If a counter is specified the
// algorithm calls Notify() once valid results have been filled into the
// response.
int SuggestBagOfWords::GetCompletions(const SuggestRequest& request,
                                      shared_ptr<SuggestResponse> response,
                                      shared_ptr<SuggestAlgoContext> context) const {
  ::util::threading::ScopedCNotify n(context->counter.get());

  VLOG(3) << "BOW: [" << request.normalized_query << "]";

  // Split the normalized query into words.
  vector<string> words;
  strutil::SplitString(request.normalized_query, " ", &words);
  if (words.empty()) return 0;

  vector<shared_ptr<SuggestResponse>> word_responses(words.size());

  // Prepare the context.
  shared_ptr<SuggestAlgoContext> w_context(new SuggestAlgoContext(*context));
  w_context->counter.reset(new ::util::threading::Counter(words.size()));

  for (int i = 0; i < words.size(); ++i) {
    // Prepare the request and the response.
    SuggestRequest w_req(request);
    w_req.normalized_query = words[i];

    // Add the space back for all but the last words. Add the space to the last word too if it too
    // has a space at the end.
    words[i] += (request.last_word_complete || i != words.size() - 1) ? " " : "";

    shared_ptr<SuggestResponse>& w_response = word_responses[i];
    w_response = shared_ptr<SuggestResponse>(new SuggestResponse);

    auto func = [&,w_req]() {
          word_suggest_algo_->GetCompletions(w_req, w_response, w_context);
        };

    if (context->pool != nullptr)  context->pool->Add(func);
    else func();
  }
  // Wait for all the completions to be filled.
  w_context->counter->Wait();

  unordered_set<SuggestionId> seen_suggestions;
  vector<std::reference_wrapper<Completion>> sort_candidates;
  for (const shared_ptr<SuggestResponse>& w_response : word_responses) {
    if (!w_response->success) continue;

    for (Completion& c : w_response->completions) {
      if (seen_suggestions.find(c.suggestion_id) != seen_suggestions.end())
        continue;
      seen_suggestions.insert(c.suggestion_id);

      double mismatch = GetWordMisMatchExtent(c.suggestion->normalized, words);
      if (mismatch < 0) continue;

      double max_mismatch = request.normalized_query.size() * c.suggestion->normalized.size();
      double mismatch_ratio = mismatch / max_mismatch;

      // double boost = max<double>(params().max_boost * (1 - mismatch_ratio), 1);
      double boost = params().max_boost * (1 - mismatch_ratio);
      VLOG(4) << "Boosting suggestion: [" << c.suggestion->normalized << "] by: "
              << boost << ", Mismatch: " << mismatch << ", Max mismatch: " << max_mismatch;

      c.debug_info.append("| BOW: Boost = " + std::to_string(boost));

      c.score *= boost;

      // Add the completion to candidates.
      sort_candidates.push_back(std::ref(c));
    }
  }

  if (sort_candidates.size()) {
    int max_suggestions = request.num_suggestions *
        params().max_suggestions_multiplier;
    partial_sort(sort_candidates.begin(),
       sort_candidates.begin() + min<int>(max_suggestions, sort_candidates.size()),
       sort_candidates.end(), rank::greater_score());

    if (sort_candidates.size() > max_suggestions)
      sort_candidates.erase(sort_candidates.begin() + max_suggestions,
                            sort_candidates.end());

    response->completions.reserve(response->completions.size() +
                                  sort_candidates.size());
    std::copy(sort_candidates.begin(), sort_candidates.end(),
              std::back_inserter(response->completions));
  }

  VLOG(4) << "BOW: Found " << response->completions.size()
         << " suggestions for [" << request.normalized_query << "]";

  response->success = true;
  return response->completions.size();
}

// Register bag of words kv index algo.
auto reg_suggest_algo_bow_index = SuggestAlgo::bind(
    "suggest_algo_bow_index", gFlag_suggest_algo_bow_index_params,
    InitializeConfigureConstructor<SuggestKeyValue, string>());

// Register bag of words algo.
auto reg_suggest_algo_bag_of_words = SuggestAlgo::bind(
    "suggest_algo_bow", gFlag_suggest_algo_bow_params,
    InitializeConfigureConstructor<SuggestBagOfWords, string>());

}  // namespace algo
}  // namespace suggest
