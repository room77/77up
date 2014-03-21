// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Suggest amenity attributes.

#include "base/defs.h"
#include "util/init/init.h"
#include "meta/suggest/server/algos/attributes/suggest_attribute.h"
#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue.h"
#include "meta/suggest/server/falcon/csmap/suggest_falcon_csmap.h"
#include "meta/suggest/util/suggest_utils.h"

// Stupid GCC 4.8 cannot parse Raw strings in macros :( ...
FLAG_string(suggest_algo_attribute_amenities_params, "{"
              "\"id\": \"amenities\","
              "\"attribute_index_algo_name\":"
                "\"suggest_algo_attribute_amenities_index\","
            "}",
            "Default parameters for suggest attribute algo.");

FLAG_string(suggest_algo_attribute_amenities_index_params, "{"
              "\"type\": " +
                  std::to_string(suggest::kCompletionAlgoTypeAttribute) + ","
              "\"falcon\": \"suggest_falcon_attribute_amenities\","
              "\"file\":"
                "\"/home/share/data/suggest/attribute/index/current/amenity/amenities_index.dat\","
            "}",
            "Parameters for suggest attribute kv_ex index algorithm.");

FLAG_string(suggest_falcon_attribute_amenities_params, "{"
            "\"id\": \"attribute_amenities\","
            "\"file\":"
              "\"/home/share/data/suggest/attribute/index/current/amenity/amenities_falcon.dat\","
            "}",
            "Default parameters for suggest attribute amenities falcon.");

namespace suggest {
namespace algo {

// Register bag of words kv index algo.
auto reg_suggest_algo_attribute_amenities_index = SuggestAlgo::bind(
    "suggest_algo_attribute_amenities_index",
    gFlag_suggest_algo_attribute_amenities_index_params,
    InitializeConfigureConstructor<SuggestSgstIdValueEx, string>());

// Register bag of words algo.
auto reg_suggest_algo_attribute_amenities = SuggestAlgo::bind(
    "suggest_algo_attribute_amenities",
    gFlag_suggest_algo_attribute_amenities_params,
    InitializeConfigureConstructor<SuggestAttribute, string>());

}  // namespace algo

namespace falcon {

// The alternate_names suggestions algorithm. This algo serves all the suggestions
// based on the alternate names.
auto reg_suggest_falcon_attribute_amenities = SuggestFalcon::bind(
    "suggest_falcon_attribute_amenities",
    gFlag_suggest_falcon_attribute_amenities_params,
    InitializeConfigureConstructor<SuggestFalconCSMap, string>());

INIT_ADD("suggest_falcon_attribute_amenities", []() {
  SuggestFalcon::pin(SuggestFalcon::make_shared("suggest_falcon_attribute_amenities"));
});

}  // namespace falcon
}  // namespace suggest
