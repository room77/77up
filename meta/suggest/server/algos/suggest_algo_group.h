// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_GROUP_H_
#define _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_GROUP_H_

#include <unordered_map>
#include <vector>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/common/merge/merge_completions.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "util/serial/serializer.h"

namespace suggest {
namespace algo {

class SuggestAlgoGroup : public SuggestAlgo {
 public:
  virtual ~SuggestAlgoGroup() {}

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
  struct SuggestAlgoGroupParams {
    struct AlgoParams {
      // The id of the algo to use.
      string id;

      // The weight of the algo.
      double weight = 1;

      // The operation to use while merging the algo.
      // This is used to initialize the merger_ object.
      // By default we pick the one with max score.
      string op = ">";

      // Whether the algo is required or not.
      // If the algo is not required, we do not necessarily wait for it to
      // finish before continuing.
      bool required = true;

      SERIALIZE(DEFAULT_CUSTOM / id*1 / weight*2 / op*3 / required*4);

      // The merger proxy to use for this algorithm.
      merge::CompletionsMerger::shared_proxy merger_;
    };

    // The id for the rank group.
    string id;

    // List of algos in the group.
    vector<AlgoParams> algo_params;

    // The timeout to wait for required algos (in milliseconds).
    int timeout_required_algos_ms = 100;

    // The additional timeout to wait for optional algos (in milliseconds).
    // NOTE: This timeout is the guaranteed extra time we wait after all the
    // required algos have finished. e.g. If the
    // required timeout = 100 ms  and optional timeout = 10, then the total time
    // we wait for optional algos will range from [0, 110] depending on when
    // the required algos finished. i.e. if the required algos finished after
    // 70msec, we wait for another 10msec for optional algos.
    int timeout_optional_algos_ms = 30;

    SERIALIZE(DEFAULT_CUSTOM / id*1 / algo_params*2 /
              timeout_required_algos_ms*3 / timeout_optional_algos_ms*4);

    // The number of required algos. This is not serialized and computed in
    // Initialize().
    int num_required_algos_ = 0;

    // The number of optional algos. This is not serialized and computed in
    // Initialize().
    int num_optional_algos_ = 0;
  };

  // Merges the suggestions from an algo
  void MergeSuggestionsFromAlgo(const SuggestRequest& request,
      const SuggestAlgoGroupParams::AlgoParams& algo_param,
      shared_ptr<SuggestResponse>& algo_response,
      unordered_map<SuggestionId, Completion>* combined_suggestions) const;

  const SuggestAlgoGroupParams& params() const { return params_; }

  // The parameters for the algorithm.
  SuggestAlgoGroupParams params_;

  // The algos in the group.
  vector<SuggestAlgo::shared_proxy> suggest_algos_;
};

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_GROUP_H_
