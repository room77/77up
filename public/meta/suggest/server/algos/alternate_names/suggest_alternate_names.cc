// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue.h"

FLAG_string(suggest_algo_alternate_names_params, "{"
            "\"type\": " +
                std::to_string(suggest::kCompletionAlgoTypeAlternateNames) + ","
            "\"falcon\": \"suggest_falcon_primary\","
            "\"file\": \"/home/share/data/suggest/locations/index/current/alternate_names/alternate_names.dat\","
            "}",
            "Default parameters for suggest alternate_names algorithm.");

namespace suggest {
namespace algo {

// The alternate_names suggestions algorithm. This algo serves all the suggestions
// based on the alternate names.
auto reg_suggest_algo_alternate_names = SuggestAlgo::bind(
    "suggest_algo_alternate_names", gFlag_suggest_algo_alternate_names_params,
    InitializeConfigureConstructor<SuggestKeyValue, string>());

}  // namespace algo
}  // namespace suggest
