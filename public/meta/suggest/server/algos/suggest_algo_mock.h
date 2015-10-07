// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_MOCK_H_
#define _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_MOCK_H_

#include "base/common.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "test/cc/test_main.h"

namespace suggest {
namespace algo {
namespace test {

// The mock class for SuggestAlgo.
class MockSuggestAlgo : public SuggestAlgo {
 public:
  // By default return true.
  bool Configure(const string& opts) { return true; }
  bool Initialize() { return true; }

  MOCK_CONST_METHOD3(GetCompletions, int(const SuggestRequest& request,
      shared_ptr<SuggestResponse> response,
      shared_ptr<SuggestAlgoContext> context));
};

// This action sets the mock suggest response whenever GetCompletions is called.
ACTION_P(SetMockSuggestResponse, response) {
  *arg1 = response;
  return response.completions.size();
}

// This action sets the mock suggest response whenever GetCompletions is called.
// In addition this also calls the counter.
ACTION_P(SetMockResponseNotifyCounter, response) {
  ::util::threading::ScopedCNotify n(arg2->counter.get());
  *arg1 = response;
  return response.completions.size();
}

inline void RegisterNewMockSuggestAlgo(const string& id,
                                       const string& params = "") {
  SuggestAlgo::bind(id, "",
      InitializeConfigureConstructor<MockSuggestAlgo, string>());
}

}  // namespace test
}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_SUGGEST_ALGO_MOCK_H_
