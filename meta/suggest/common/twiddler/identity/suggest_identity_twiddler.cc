// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "base/common.h"
#include "meta/suggest/common/twiddler/suggest_twiddler.h"
#include "meta/suggest/common/suggest_datatypes.h"

FLAG_string(suggest_twiddler_identity_params, "",
            "Default parameters for suggest domain boost twiddler algorithm.");

namespace suggest {
namespace twiddle {

// The twiddler for domain boost.
class SuggestIdentityTwiddler : public SuggestTwiddler {
 public:
  virtual ~SuggestIdentityTwiddler() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  virtual bool GetScore(const SuggestTwiddleRequest& request,
                        shared_ptr<SuggestTwiddleResponse>& response,
                        shared_ptr<SuggestTwiddlerContext> context) const {
    ::util::threading::ScopedCNotify n(context->counter.get());

    if (!request.suggest_response->success ||
        request.suggest_response->completions.empty()) return false;

    suggest::twiddle::SuggestTwiddleResponse::CompletionScore val;
    val.score = 1;
    response->completion_scores.resize(
        request.suggest_response->completions.size(), val);
    response->success = true;
    return true;
  }
};

// The domain boost twiddler.
auto reg_suggest_twiddler_identity = SuggestTwiddler::bind(
    "suggest_twiddler_identity",
    InitializeConfigureConstructor<SuggestIdentityTwiddler, string>(
        gFlag_suggest_twiddler_identity_params));

}  // namespace twiddle
}  // namespace suggest
