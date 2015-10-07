// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/suggest/server/falcon/csmap/suggest_falcon_csmap.h"
#include "util/init/init.h"

FLAG_string(suggest_falcon_primary_params, "{"
            "\"id\": \"primary\","
            "\"file\": \"/home/share/data/suggest/locations/index/current/falcon/suggest_index.dat\","
            "}",
            "Default parameters for suggest primary falcon.");

namespace suggest {
namespace falcon {

// The alternate_names suggestions algorithm. This algo serves all the suggestions
// based on the alternate names.
auto reg_suggest_falcon_primary = SuggestFalcon::bind(
    "suggest_falcon_primary", gFlag_suggest_falcon_primary_params,
    InitializeConfigureConstructor<SuggestFalconCSMap, string>());

INIT_ADD("suggest_falcon_primary", [](){
  SuggestFalcon::pin(SuggestFalcon::make_shared("suggest_falcon_primary"));
});

}  // namespace falcon
}  // namespace suggest
