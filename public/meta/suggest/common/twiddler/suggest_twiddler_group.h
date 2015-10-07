// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_COMMON_TWIDDLER_SUGGEST_TWIDDLER_GROUP_H_
#define _META_SUGGEST_COMMON_TWIDDLER_SUGGEST_TWIDDLER_GROUP_H_

#include <vector>

#include "base/common.h"
#include "meta/rank/operator.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/common/twiddler/suggest_twiddler.h"
#include "util/serial/serializer.h"

namespace suggest {
namespace twiddle {

// Generic twiddle to combine a group of twiddlers and return their combined
// scores.
class SuggestTwiddlerGroup : public SuggestTwiddler {
 public:
  virtual ~SuggestTwiddlerGroup() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return params_.FromJSON(opts); }

  // Initialize the class.
  virtual bool Initialize();

  virtual bool GetScore(const SuggestTwiddleRequest& request,
                        shared_ptr<SuggestTwiddleResponse>& response,
                        shared_ptr<SuggestTwiddlerContext> context) const;

 protected:
  // The parameters struct to configure the twiddler.
  struct SuggestTwiddlerGroupParams {
    struct TwiddlerParams {
      // The id of the twiddler to use.
      string id;

      // The weight of the twiddler.
      double weight = 1;

      // The operation to use while combining the twiddler.
      string op = "*";

      // Whether the twiddler is required or not.
      // If the twiddler is not required, we do not necessarily wait for it to
      // finish before continuing.
      bool required = true;

      SERIALIZE(DEFAULT_CUSTOM / id*1 / weight*2 / op*3 / required*4);

      // The merger proxy to use for this twiddler.
      ::meta::rank::Operator::shared_proxy operator_;
    };

    // The id for the rank group.
    string id;

    // List of twiddlers in the group.
    vector<TwiddlerParams> twiddler_params;

    // The timeout to wait for required algos (in milliseconds).
    int timeout_required_twiddlers_ms = 100;

    // The additional timeout to wait for optional twiddlers (in milliseconds).
    // NOTE: This timeout is the guaranteed extra time we wait after all the
    // required twiddlers have finished. e.g. If the
    // required timeout = 100 ms  and optional timeout = 10, then the total time
    // we wait for optional twiddlers will range from [0, 110] depending on when
    // the required algos finished. i.e. if the required twiddlers finished after
    // 70msec, we wait for another 10msec for optional twiddlers.
    int timeout_optional_twiddlers_ms = 30;

    SERIALIZE(DEFAULT_CUSTOM / id*1 / twiddler_params*2 /
              timeout_required_twiddlers_ms*3 / timeout_optional_twiddlers_ms*4);

    // The number of required twiddlers. This is not serialized and computed in
    // Initialize().
    int num_required_twiddlers_ = 0;

    // The number of optional twiddlers. This is not serialized and computed in
    // Initialize().
    int num_optional_twiddlers_ = 0;
  };

  // Combine the twiddler score.
  void CombineTwiddlerScore(
      const SuggestTwiddlerGroupParams::TwiddlerParams& twiddler_param,
      shared_ptr<SuggestTwiddleResponse>& twiddler_response,
      SuggestTwiddleResponse* combined_response) const;

  const SuggestTwiddlerGroupParams& params() const { return params_; }

  // The parameters for the twiddler.
  SuggestTwiddlerGroupParams params_;

  // The twiddlers in the group.
  vector<SuggestTwiddler::shared_proxy> suggest_twiddlers_;
};

}  // namespace twiddle
}  // namespace suggest


#endif  // _META_SUGGEST_COMMON_TWIDDLER_SUGGEST_TWIDDLER_GROUP_H_
