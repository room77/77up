// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/suggest/server/algos/suggest_algo_group.h"

FLAG_string(suggest_algo_primary_group_params,
            "{"
              "\"id\": \"suggest_algo_primary_group\","
              "\"algo_params\": ["
                "{"
                  "\"id\": \"suggest_algo_prefix\","
                  "\"weight\": 100,"
                  "\"required\": true"
                "}, {"
                  "\"id\": \"suggest_algo_alternate_names\","
                  "\"weight\": 2,"
                  "\"op\": \"+\","
                  "\"required\": false"
                "}, {"
                  "\"id\": \"suggest_algo_bow\","
                  "\"weight\": 1,"
                  "\"op\": \"+\","
                  "\"required\": false"
                "}"
              "]"
            "}",
            "Parameters for primary group of algos.");

FLAG_string(suggest_algo_primary_group_params_dbg,
            "{"
              "\"id\": \"suggest_algo_primary_group\","
              "\"algo_params\": ["
                "{"
                  "\"id\": \"suggest_algo_prefix\","
                  "\"weight\": 100,"
                  "\"required\": true"
                "}"
              "]"
            "}",
            "Parameters for primary group of algos.");

FLAG_string(suggest_algo_secondary_group_params,
            "{"
              "\"id\": \"suggest_algo_secondary_group\","
              "\"algo_params\": ["
                "{"
                  "\"id\": \"suggest_algo_attribute_filters\","
                  "\"weight\": 1,"
                  "\"required\": true"
                "}, {"
                  "\"id\": \"suggest_algo_attribute_sorts\","
                  "\"weight\": 0.35,"
                  "\"required\": true"
                "}, {"
                  "\"id\": \"suggest_algo_attribute_attractions\","
                  "\"weight\": 0.4,"
                  "\"required\": true"
                "}, {"
                  "\"id\": \"suggest_algo_attribute_neighborhoods\","
                  "\"weight\": 0.4,"
                  "\"required\": true"
                "}, {"
                  "\"id\": \"suggest_algo_attribute_amenities\","
                  "\"weight\": 1,"
                  "\"required\": true"
                "}"
              "]"
            "}",
            "Parameters for secondary group of algos.");

namespace suggest {
namespace algo {

// Register suggest algo primary group.
auto reg_suggest_algo_primary_group = SuggestAlgo::bind(
    "suggest_algo_primary_group", gFlag_suggest_algo_primary_group_params,
    InitializeConfigureConstructor<SuggestAlgoGroup, string>());

// Register suggest algo primary group.
auto reg_suggest_algo_secondary_group = SuggestAlgo::bind(
    "suggest_algo_secondary_group", gFlag_suggest_algo_secondary_group_params,
    InitializeConfigureConstructor<SuggestAlgoGroup, string>());

}  // namespace algo
}  // namespace suggest
