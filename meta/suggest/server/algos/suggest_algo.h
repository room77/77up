// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_H_
#define _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_H_

#include <memory>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/factory/factory.h"
#include "util/factory/factory_extra.h"

namespace suggest {
namespace algo {

// This is the context that an algo can use to share different parameters with
// caller and other algos.
struct SuggestAlgoContext : public SuggestContext {};

// The interface class for different suggest algorithms.
class SuggestAlgo : public Factory<SuggestAlgo, string, string> {
 public:
  virtual ~SuggestAlgo() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  // The interface function for different suggest Algos. The response is filled
  // with relevant results from the algorithm.
  // If a counter is specified in the context, the algorithm calls Notify()
  // once valid results have been filled into the response.
  // Notes:
  // 1. The response must have 'success' set if an algo returns valid data.
  // 2. The response and context object may never be null.
  virtual int GetCompletions(const SuggestRequest& request,
                             shared_ptr<SuggestResponse> response,
                             shared_ptr<SuggestAlgoContext> context) const = 0;

  // Utility function when there is no context specified.
  int GetCompletions(const SuggestRequest& request,
                             shared_ptr<SuggestResponse> response) const {
    return GetCompletions(request, response,
                          shared_ptr<SuggestAlgoContext>(new SuggestAlgoContext));
  }
};

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_H_
