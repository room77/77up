// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/init/init.h"
#include "meta/suggest/server/suggestions/suggestion_manager.h"
#include "meta/suggest/server/suggestions/suggestions.h"

FLAG_string(suggestion_manager_primary_algo, "suggest_algo_primary_group",
            "The primary suggestion algo to use.");

FLAG_string(suggestion_manager_fallback_algo, "suggest_algo_fallback",
            "The fallback suggestion algo to use when primary algo does not"
            "return enough suggestions.");

FLAG_string(suggestion_manager_secondary_algo, "suggest_algo_secondary_group",
            "The secondary suggestion algo to use after the primary and fallback"
            "algorithms have returned.");

FLAG_string(suggestion_manager_dedupers, "suggest_dedup_duplicate",
            "Comma separated list of dedupers to use.");

FLAG_string(suggestion_manager_primary_twiddler, "suggest_twiddler_primary_group",
            "The twiddler to use to rerank primary suggestions.");

FLAG_string(suggestion_manager_secondary_twiddler,
            "suggest_twiddler_secondary_group",
            "The twiddler to use to rerank secondary suggestions.");

FLAG_int(suggestion_manager_threadpool_size, 512,
         "Number of threads to use for the suggest thread pool.");

namespace suggest {

int SuggestionManager::GetCompletions(SuggestRequestInterface& request,
                                      shared_ptr<SuggestResponse> response) {
  // In future, we may want to do this through a pool if we make the reply
  // asynchronous.
  Suggestions suggestions(this);
  return suggestions.GetCompletions(request, response);
}

// -------------- INITIALIZERS --------------

void SuggestionManager::Initialize() {
  LOG(INFO) << "Initializing SuggestionManager.";
  InitPrimaryAlgo();
  InitFallbackAlgo();
  InitSecondaryAlgo();
  InitDedupers();
  InitPrimaryTwiddler();
  InitSecondaryTwiddler();
  InitThreadPool();
  LOG(INFO) << "SuggestionManager initialized.";
}

void SuggestionManager::InitPrimaryAlgo() {
  primary_algo_ = algo::SuggestAlgo::make_shared(
      gFlag_suggestion_manager_primary_algo);
  ASSERT_NOTNULL(primary_algo_) << "Could not initialize algo "
      << gFlag_suggestion_manager_primary_algo;
}

void SuggestionManager::InitFallbackAlgo() {
  fallback_algo_ = algo::SuggestAlgo::make_shared(
      gFlag_suggestion_manager_fallback_algo);
  ASSERT_NOTNULL(fallback_algo_) << "Could not initialize algo "
      << gFlag_suggestion_manager_fallback_algo;
}

void SuggestionManager::InitSecondaryAlgo() {
  secondary_algo_ = algo::SuggestAlgo::make_shared(
      gFlag_suggestion_manager_secondary_algo);
  ASSERT_NOTNULL(secondary_algo_) << "Could not initialize algo "
      << gFlag_suggestion_manager_secondary_algo;
}

void SuggestionManager::InitDedupers() {
  vector<string> deduper_ids;
  strutil::SplitString(gFlag_suggestion_manager_dedupers, ",", &deduper_ids);

  dedupers_.reserve(deduper_ids.size());
  for (const string& id : deduper_ids) {
    dedup::SuggestDedup::shared_proxy deduper = dedup::SuggestDedup::make_shared(id);
    ASSERT_NOTNULL(deduper) << "Could not initialize deduper "
        << id;
    dedupers_.push_back(deduper);
  }
}

void SuggestionManager::InitPrimaryTwiddler() {
  primary_twiddler_ = twiddle::SuggestTwiddler::make_shared(
      gFlag_suggestion_manager_primary_twiddler);
  ASSERT_NOTNULL(primary_twiddler_) << "Could not initialize primary twiddler "
      << gFlag_suggestion_manager_primary_twiddler;
}

void SuggestionManager::InitSecondaryTwiddler() {
  secondary_twiddler_ = twiddle::SuggestTwiddler::make_shared(
      gFlag_suggestion_manager_secondary_twiddler);
  ASSERT_NOTNULL(secondary_twiddler_) << "Could not initialize secondary twiddler "
      << gFlag_suggestion_manager_secondary_twiddler;
}

void SuggestionManager::InitThreadPool() {
  thread_pool_.reset(new ::util::threading::ThreadPool(
      gFlag_suggestion_manager_threadpool_size));

  ASSERT_NOTNULL(thread_pool_) << "Could not initialize suggest thread pool";
}

INIT_ADD("suggestion_manager", [](){ SuggestionManager::Instance(); });

}  // namespace suggest
