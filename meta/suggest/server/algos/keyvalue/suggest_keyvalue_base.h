// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Generic key value algorithm that loads a file and serves the index.


#ifndef _META_SUGGEST_SERVER_ALGOS_KEYVALUE_SUGGEST_KEYVALUE_BASE_H_
#define _META_SUGGEST_SERVER_ALGOS_KEYVALUE_SUGGEST_KEYVALUE_BASE_H_

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "meta/suggest/server/falcon/suggest_falcon.h"
#include "util/serial/serializer.h"

namespace suggest {
namespace algo {

// Base class for different key value algorithms.
class SuggestKeyValueBase : public SuggestAlgo {
 public:
  virtual ~SuggestKeyValueBase() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return params_.FromJSON(opts); }

  // Initialize the class.
  virtual bool Initialize();

  // The interface function for different suggest Algos. The response is filled
  // with relevant results from the algorithm. If a counter is specified the
  // algorithm calls Notify() once valid results have been filled into the
  // response.
  // Currently, the only requirement on the request is that normalized_query
  // must be set.
  virtual int GetCompletions(const SuggestRequest& request,
                             shared_ptr<SuggestResponse> response,
                             shared_ptr<SuggestAlgoContext> context) const;

 protected:
  // All subclasses must implement this function to fill the response with the
  // completions corresponding to the request.
  virtual int FindCompletions(const SuggestRequest& request,
                              shared_ptr<SuggestResponse> response,
                              shared_ptr<SuggestAlgoContext> context) const = 0;

  // The parameters struct to configure the algo.
  struct SuggestKeyValueParams {
    // The algo type to use.
    SugggestionAlgoType type = kCompletionAlgoTypeInvalid;

    // The file name to read the index from.
    string file;

    string falcon;
    SERIALIZE(DEFAULT_CUSTOM / type*1 / file*2 / falcon*4);

    // The name of the algo computed based on the type.
    string name;
  };

  const SuggestKeyValueParams& params() const { return params_; }

  // The parameters for the algorithm.
  SuggestKeyValueParams params_;

  falcon::SuggestFalcon::shared_proxy falcon_;
};

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_KEYVALUE_SUGGEST_KEYVALUE_BASE_H_
