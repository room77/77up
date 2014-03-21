// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The fallback algorithm to use when the primary algo fails to return any
// suggestions.


#ifndef _META_SUGGEST_SERVER_ALGOS_FALLBACK_SUGGEST_FALLBACK_H_
#define _META_SUGGEST_SERVER_ALGOS_FALLBACK_SUGGEST_FALLBACK_H_

#include <unordered_map>
#include <vector>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "util/serial/serializer.h"

namespace suggest {
namespace algo {

class SuggestFallback : public SuggestAlgo {
 public:
  virtual ~SuggestFallback() {}

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
  struct SuggestFallbackParams {
    // The file name to read the index from.
    string file;
    SERIALIZE(DEFAULT_CUSTOM / file*1);

    // The name of the algo computed based on the type.
    string name;
  };

  const SuggestFallbackParams& params() const { return params_; }

  // The parameters for the algorithm.
  SuggestFallbackParams params_;
};

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_FALLBACK_SUGGEST_FALLBACK_H_
