// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue.h"

FLAG_string(suggest_algo_prefix_params, "{"
            "\"type\": " +
                std::to_string(suggest::kCompletionAlgoTypePrefix) + ","
            "\"falcon\": \"suggest_falcon_primary\","
            "\"file\": \"/home/share/data/suggest/locations/index/current/prefix/prefix.dat\","
            "}",
            "Default parameters for suggest prefix algorithm.");

namespace suggest {
namespace algo {

// The prefix suggestions algorithm. This algorithm typically serves the bulk
// of suggestions.
auto reg_suggest_algo_prefix = SuggestAlgo::bind(
    "suggest_algo_prefix", gFlag_suggest_algo_prefix_params,
    InitializeConfigureConstructor<SuggestKeyValue, string>());

}  // namespace algo
}  // namespace suggest
