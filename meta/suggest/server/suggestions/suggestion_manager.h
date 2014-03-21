// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_SUGGESTIONS_SUGGESTION_MANAGER_H_
#define _META_SUGGEST_SERVER_SUGGESTIONS_SUGGESTION_MANAGER_H_

#include <memory>

#include "base/common.h"

#include "meta/suggest/common/dedup/suggest_dedup.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/common/twiddler/suggest_twiddler.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "util/thread/thread_pool.h"

namespace suggest {

// Class for managing stateful data required across multiple suggestion requests.
class SuggestionManager {
 public:
  virtual ~SuggestionManager() {}

  static SuggestionManager* Instance() {  // singleton instance
    struct MakeInstance {
      SuggestionManager operator()() {
        SuggestionManager instance;
        instance.Initialize();
        return instance;
      }
    };
    static SuggestionManager instance = MakeInstance()();
    return &instance;
  }

  // Returns the completions for a given suggest request.
  int GetCompletions(SuggestRequestInterface& request,
                     shared_ptr<SuggestResponse> response);


  virtual algo::SuggestAlgo::shared_proxy& primary_algo() {
    return primary_algo_;
  }

  virtual algo::SuggestAlgo::shared_proxy& fallback_algo() {
    return fallback_algo_;
  }

  virtual algo::SuggestAlgo::shared_proxy& secondary_algo() {
    return secondary_algo_;
  }

  virtual vector<dedup::SuggestDedup::shared_proxy>& dedupers() {
    return dedupers_;
  }

  virtual twiddle::SuggestTwiddler::shared_proxy& primary_twiddler() {
    return primary_twiddler_;
  }

  virtual twiddle::SuggestTwiddler::shared_proxy& secondary_twiddler() {
    return secondary_twiddler_;
  }

  virtual ::util::threading::ThreadPool* thread_pool() {
    return thread_pool_.get();
  }

 protected:
  SuggestionManager() {}

  // Initialize the Suggestion manager.
  virtual void Initialize();

  // Initialize individual components.
  virtual void InitPrimaryAlgo();
  virtual void InitFallbackAlgo();
  virtual void InitSecondaryAlgo();
  virtual void InitDedupers();
  virtual void InitPrimaryTwiddler();
  virtual void InitSecondaryTwiddler();
  virtual void InitThreadPool();

  // The primary suggest algo to fetch suggestions.
  // This will typically be an algo group fetching suggestions from multiple
  // algos.
  algo::SuggestAlgo::shared_proxy primary_algo_;

  // This is the fallback algo to use to fetch results when the primary algo
  // cannot fetch enough suggestions.
  algo::SuggestAlgo::shared_proxy fallback_algo_;

  // The secondary suggest algo to fetch more suggestion after the
  // primary/fallback algos have already have already fetched suggestions.
  // This typically requires knowledge of suggestions already fetched by other
  // algorithms.
  algo::SuggestAlgo::shared_proxy secondary_algo_;

  // A list of dedupers run in order one after the other to dedup suggestions.
  vector<dedup::SuggestDedup::shared_proxy> dedupers_;

  // The primary twiddler used to rerank all the primary suggestions before they
  // are sorted and deduped.
  twiddle::SuggestTwiddler::shared_proxy primary_twiddler_;

  // The secondary twiddler used to rerank all the secondary suggestions before
  // they are sorted and deduped.
  twiddle::SuggestTwiddler::shared_proxy secondary_twiddler_;

  // The common thread pool to use.
  shared_ptr< ::util::threading::ThreadPool> thread_pool_;
};

}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_SUGGESTIONS_SUGGESTION_MANAGER_H_
