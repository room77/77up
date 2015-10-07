// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/serial/serializer.h"

namespace suggest {

class DummyAttr {
 public:
  void Gen() {
    PopulateFalcon();
    PopulateIndex();
    DumpIndexFile();
    DumpFalconFile();
  }

 private:

  void PopulateFalcon() {
    falcon_["f/BeachFront"] = GetDummyAttribute("f/BeachFront", " hotels near the beach");
    falcon_["f/UserRating3a"] =
        GetDummyAttribute("f/UserRating3a", "hotels with user rating 3 and above");
    falcon_["f/Star4a"] =
        GetDummyAttribute("f/Star4a", "hotels with 4 stars or above");

    falcon_["r/cheap"] = GetDummyAttribute("r/cheap", "cheap hotels");
    falcon_["r/BestUserRating"] =
        GetDummyAttribute("r/BestUserRating", "hotels with best user ratings");

    falcon_["am/FreeWiFi"] = GetDummyAttribute("am/FreeWiFi", "hotels with free wifi");
    falcon_["am/BusinessCenter"] = GetDummyAttribute("am/BusinessCenter",
                                                     "hotels with business center");
    falcon_["am/FitnessCenter"] = GetDummyAttribute("am/FitnessCenter",
                                                    "hotels with fitness center");
    falcon_["am/Parking"] = GetDummyAttribute("am/Parking", "hotels with parking");
  }

  void PopulateIndex() {
    kv_map_["m/default_order"] = {
         {CompletionIndexItemEx("r/BestUserRating")},
         {CompletionIndexItemEx("r/cheap")},
         {CompletionIndexItemEx("f/Star4a")},
         {CompletionIndexItemEx("am/FreeWiFi")},
        };

    // San Francisco, CA
    kv_map_["c/US:3103989074"] = {
         {CompletionIndexItemEx("f/Star4a", 0.4)},
         {CompletionIndexItemEx("am/FitnessCenter", 0.3)},
         {CompletionIndexItemEx("f/UserRating3a", 0.2)},
         {CompletionIndexItemEx("r/cheap", 0.1)},
         {CompletionIndexItemEx("am/FreeWiFi", 0.05)},
        };

    // New York, NY
    kv_map_["c/US:4145841071"] = {
         {CompletionIndexItemEx("r/cheap", 0.3)},
         {CompletionIndexItemEx("am/Parking", 0.2)},
         {CompletionIndexItemEx("f/Star4a", 0.4)},
         {CompletionIndexItemEx("am/BusinessCenter", 0.1)},
         {CompletionIndexItemEx("r/BestUserRating", 0.09)},
        };

    // Miami, FL
    kv_map_["c/US:3016190638"] = {
         {CompletionIndexItemEx("f/BeachFront", 0.5)},
         {CompletionIndexItemEx("r/cheap", 0.3)},
         {CompletionIndexItemEx("r/BestUserRating", 0.3)},
         {CompletionIndexItemEx("am/FreeWiFi", 0.1)},
        };

    // Seattle, WA
    kv_map_["c/US:1218578969"] = {
         {CompletionIndexItemEx("r/cheap", 0.2)},
         {CompletionIndexItemEx("r/BestUserRating", 0.1)},
         {CompletionIndexItemEx("am/BusinessCenter", 0.05)},
         {CompletionIndexItemEx("am/FreeWiFi", 0.04)},
        };
  }

  shared_ptr<CompleteSuggestion> GetDummyAttribute(const string& id,
                                                   const string& name) {
    static int counter = 0;
    shared_ptr<CompleteSuggestion> suggestion(new CompleteSuggestion);
    suggestion->src_type = ::entity::kEntityTypeFilter;
    suggestion->src_id = id;

    suggestion->base_score = max(10.0, (50.0 - (++counter) * 5)) / 100;
    suggestion->freq = 100 - counter;

    suggestion->display = name;
    suggestion->normalized = name;

    return suggestion;
  }

  void DumpIndexFile() {
    static const string file =
        "/tmp/suggest/dummy_index.dat";

    ofstream f(file.c_str());
    serial::Serializer::ToBinary(f, kv_map_);
  }

  void DumpFalconFile() {
    static const string file =
        "/tmp/suggest/dummy_falcon.dat";

    ofstream f(file.c_str());
    serial::Serializer::ToBinary(f, falcon_);
  }

  unordered_map<SuggestionId, vector<CompletionIndexItemEx>> kv_map_;
  unordered_map<SuggestionId, shared_ptr<CompleteSuggestion>> falcon_;
};

}  // namespace suggest


int init_main() {
  suggest::DummyAttr attr;
  attr.Gen();

  return 0;
}
