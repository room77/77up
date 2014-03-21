// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "base/common.h"
#include "domain_boost.h"
#include "meta/suggest/common/twiddler/suggest_twiddler.h"
#include "meta/suggest/common/suggest_datatypes.h"

FLAG_string(suggest_twiddler_domain_boost_params, "",
            "Default parameters for suggest domain boost twiddler algorithm.");

namespace suggest {
namespace twiddle {

// The twiddler for domain boost.
class SuggestDomainBoostTwiddler : public SuggestTwiddler {
 public:
  virtual ~SuggestDomainBoostTwiddler() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  virtual bool GetScore(const SuggestTwiddleRequest& request,
                        shared_ptr<SuggestTwiddleResponse>& response,
                        shared_ptr<SuggestTwiddlerContext> context) const {
    ::util::threading::ScopedCNotify n(context->counter.get());

    // TODO(pramodg):use const&.
    geo::domain_boost::DomainBoost& domain_boost =
        geo::domain_boost::DomainBoost::Instance();

    response->completion_scores.reserve(request.suggest_response->completions.size());
    for (const Completion& completion : request.suggest_response->completions) {
      SuggestTwiddleResponse::CompletionScore score;
      score.score = domain_boost.Boost(request.suggest_request.user_country,
                                        completion.suggestion->country);
      response->completion_scores.push_back(score);
    }
    response->success = true;
    return true;
  }
};

// The domain boost twiddler.
auto reg_suggest_twiddler_domain_boost = SuggestTwiddler::bind(
    "suggest_twiddler_domain_boost",
    InitializeConfigureConstructor<SuggestDomainBoostTwiddler, string>(
        gFlag_suggest_twiddler_domain_boost_params));

}  // namespace twiddle
}  // namespace suggest
