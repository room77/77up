// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The prefix suggestions algorithm. This algorithm typically serves the bulk
// of suggestions.


#ifndef _META_SUGGEST_SERVER_ALGOS_BAG_OF_WORDS_SUGGEST_BAG_OF_WORDS_H_
#define _META_SUGGEST_SERVER_ALGOS_BAG_OF_WORDS_SUGGEST_BAG_OF_WORDS_H_

#include <unordered_map>
#include <vector>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "util/serial/serializer.h"

namespace suggest {
namespace algo {

class SuggestBagOfWords : public SuggestAlgo {
 public:
  virtual ~SuggestBagOfWords() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return params_.FromJSON(opts); }

  // Initialize the class.
  virtual bool Initialize();

  // The interface function for different suggest Algos. The response is filled
  // with relevant results from the algorithm. If a counter is specified the
  // algorithm calls Notify() once valid results have been filled into the
  // response.
  virtual int GetCompletions(const SuggestRequest& request,
                             shared_ptr<SuggestResponse> response,
                             shared_ptr<SuggestAlgoContext> context) const;

 protected:
  // The parameters struct to configure the algo.
  struct SuggestBagOfWordsParams {
    // The algo name to use to fetch suggestions for each word.
    string word_suggest_algo_name;

    // The max suggestions that the algorithm can return. This computed by
    // mulitplying the request.num_suggestions with the multiplier.
    int max_suggestions_multiplier = 7;

    // The max boost a suggestion can get based on the extent of match with the
    // requested query.
    int max_boost = 5;

    SERIALIZE(DEFAULT_CUSTOM / word_suggest_algo_name*1 /
              max_suggestions_multiplier*2 / max_boost*3);
  };

  const SuggestBagOfWordsParams& params() const { return params_; }

  // The parameters for the algorithm.
  SuggestBagOfWordsParams params_;

  // The index for the suggest prefixes.
  shared_proxy word_suggest_algo_;
};

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_BAG_OF_WORDS_SUGGEST_BAG_OF_WORDS_H_
