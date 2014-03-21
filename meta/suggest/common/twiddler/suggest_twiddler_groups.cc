// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/suggest/common/twiddler/suggest_twiddler_group.h"

FLAG_string(suggest_twiddler_primary_group_params,
            "{"
              "\"id\": \"suggest_twiddler_primary_group\","
              "\"twiddler_params\": ["
                "{"
                  "\"id\": \"suggest_twiddler_domain_boost\","
                  "\"weight\": 1,"
                  "\"required\": true"
                "}"
              "]"
            "}",
            "Parameters for primary group of twiddlers.");

FLAG_string(suggest_twiddler_secondary_group_params,
            "{"
              "\"id\": \"suggest_twiddler_secondary_group\","
              "\"twiddler_params\": ["
                "{"
                  "\"id\": \"suggest_twiddler_identity\","
                  "\"weight\": 1,"
                  "\"required\": true"
                "}"
              "]"
            "}",
            "Parameters for secondary group of twiddlers.");

namespace suggest {
namespace twiddle {

// Register suggest algo primary group.
auto reg_suggest_twiddler_primary_group = SuggestTwiddler::bind(
    "suggest_twiddler_primary_group",
    InitializeConfigureConstructor<SuggestTwiddlerGroup, string>(
        gFlag_suggest_twiddler_primary_group_params));

// Register suggest algo primary group.
auto reg_suggest_twiddler_secondary_group = SuggestTwiddler::bind(
    "suggest_twiddler_secondary_group",
    InitializeConfigureConstructor<SuggestTwiddlerGroup, string>(
        gFlag_suggest_twiddler_secondary_group_params));

}  // namespace twiddle
}  // namespace suggest
