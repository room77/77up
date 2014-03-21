// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Suggest neighborhood attributes.

#include "base/defs.h"
#include "util/init/init.h"
#include "meta/suggest/server/algos/attributes/location/suggest_attribute_location.h"
#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue.h"
#include "meta/suggest/server/falcon/csmap/suggest_falcon_csmap.h"
#include "meta/suggest/util/suggest_utils.h"

// Stupid GCC 4.8 cannot parse Raw strings in macros :( ...
FLAG_string(suggest_algo_attribute_neighborhoods_params, "{"
              "\"id\": \"neighborhoods\","
              "\"attribute_index_algo_name\":"
                "\"suggest_algo_attribute_neighborhoods_index\","
            "}",
            "Default parameters for suggest attribute algo.");

FLAG_string(suggest_algo_attribute_neighborhoods_index_params, "{"
              "\"type\": " +
                  std::to_string(suggest::kCompletionAlgoTypeAttribute) + ","
              "\"falcon\": \"suggest_falcon_attribute_neighborhoods\","
              "\"file\":"
                "\"/home/share/data/suggest/attribute/index/current/neighborhood/neighborhoods_index.dat\","
            "}",
            "Parameters for suggest attribute neighborhoods kv_ex index algorithm.");

FLAG_string(suggest_falcon_attribute_neighborhoods_params, "{"
            "\"id\": \"attribute_neighborhoods\","
            "\"file\":"
              "\"/home/share/data/suggest/attribute/index/current/neighborhood/neighborhoods_falcon.dat\","
            "}",
            "Default parameters for suggest attribute neighborhoods falcon.");

namespace suggest {
namespace algo {

// Register bag of words kv index algo.
auto reg_suggest_algo_attribute_neighborhoods_index = SuggestAlgo::bind(
    "suggest_algo_attribute_neighborhoods_index",
    gFlag_suggest_algo_attribute_neighborhoods_index_params,
    InitializeConfigureConstructor<SuggestSgstIdValueEx, string>());

// Register bag of words algo.
auto reg_suggest_algo_attribute_neighborhoods = SuggestAlgo::bind(
    "suggest_algo_attribute_neighborhoods",
    gFlag_suggest_algo_attribute_neighborhoods_params,
    InitializeConfigureConstructor<SuggestLocationAttribute, string>());

}  // namespace algo

namespace falcon {

// The alternate_names suggestions algorithm. This algo serves all the suggestions
// based on the alternate names.
auto reg_suggest_falcon_attribute_neighborhoods = SuggestFalcon::bind(
    "suggest_falcon_attribute_neighborhoods",
    gFlag_suggest_falcon_attribute_neighborhoods_params,
    InitializeConfigureConstructor<SuggestFalconCSMap, string>());

INIT_ADD("suggest_falcon_attribute_neighborhoods", []() {
  SuggestFalcon::pin(SuggestFalcon::make_shared(
      "suggest_falcon_attribute_neighborhoods"));
});

}  // namespace falcon
}  // namespace suggest
