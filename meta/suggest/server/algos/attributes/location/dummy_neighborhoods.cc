// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "util/file/file.h"
#include "base/defs.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/region_data/neighborhood/neighborhoods.h"
#include "util/region_data/utils.h"
#include "util/serial/serializer.h"

FLAG_string(output_falcon_file,
            "/home/share/datn/suggest/attribute/index/current/neighborhood/neighborhoods_index.dat",
            "The falcon file to be generated.");

FLAG_string(output_index_file,
            "/home/share/datn/suggest/attribute/index/current/neighborhood/neighborhoods_index.dat",
            "The index file to be generated.");

namespace suggest {

class DummyNeighborhoods {
 public:
  void Gen() {
//    PopulateFalcon();
//    PopulateIndex();
    DumpIndexFile();
    DumpFalconFile();
  }

 private:
  void PopulateFalcon() {
    // San Francisco, CA
    falcon_["n/2191"] = GetDummyNeighborhood("n/2191");  // North Beach
    falcon_["n/2142"] = GetDummyNeighborhood("n/2142");  // Chinatown
    falcon_["n/2152"] = GetDummyNeighborhood("n/2152");  // Financial District
    falcon_["n/2138"] = GetDummyNeighborhood("n/2138");  // Castro

    // Boston, MA
    falcon_["n/5448"] = GetDummyNeighborhood("n/5448");  // Back Bay
    falcon_["n/21525"] = GetDummyNeighborhood("n/21525");  // Downtown
    falcon_["n/5464"] = GetDummyNeighborhood("n/5464");  // North End

    // New York, NY
    falcon_["n/14764"] = GetDummyNeighborhood("n/14764");  // Downtown Manhattan
    falcon_["n/7657"] = GetDummyNeighborhood("n/7657");  // Theater District - Times Square
    falcon_["n/7661"] = GetDummyNeighborhood("n/7661");  // Upper East Side

    // Chicago, IL
    falcon_["n/4493"] = GetDummyNeighborhood("n/4493");  // The Loop
    falcon_["n/4483"] = GetDummyNeighborhood("n/4483");  // Hyde Park
    falcon_["n/4489"] = GetDummyNeighborhood("n/4489");  // Lincoln Park
    falcon_["n/66269"] = GetDummyNeighborhood("n/66269");  // Gold Coast

    // Miami, FL
    falcon_["n/3585"] = GetDummyNeighborhood("n/3585");  // Downtown Miami
    falcon_["n/141074"] = GetDummyNeighborhood("n/141074");  // Design District
    falcon_["n/146618"] = GetDummyNeighborhood("n/146618");  // Coconut Grove Park

    // Vegas, NV
    falcon_["n/85108"] = GetDummyNeighborhood("n/85108");  // The Strip
    falcon_["n/65009"] = GetDummyNeighborhood("n/65009");  // Downtown
    falcon_["n/7408"] = GetDummyNeighborhood("n/7408");  // Summerlin
  }

  void PopulateIndex() {
    // San Francisco, CA
    kv_map_["c/san francisco,CA,US"] = {
         {CompletionIndexItemEx("n/2191", 0.99)},
         {CompletionIndexItemEx("n/2142", 0.7)},
         {CompletionIndexItemEx("n/2152", 0.6)},
         {CompletionIndexItemEx("n/2138", 0.5)},
        };

    // Boston, MA
    kv_map_["c/boston,MA,US"] = {
         {CompletionIndexItemEx("n/5448", 0.99)},
         {CompletionIndexItemEx("n/21525", 0.8)},
         {CompletionIndexItemEx("n/5464", 0.7)},
        };

    // New York, NY
    kv_map_["c/new york,NY,US"] = {
         {CompletionIndexItemEx("n/14764", 0.99)},
         {CompletionIndexItemEx("n/7657", 0.6)},
         {CompletionIndexItemEx("n/7661", 0.5)},
        };

    // Chicago, IL
    kv_map_["c/chicago,IL,US"] = {
         {CompletionIndexItemEx("n/4493", 0.99)},
         {CompletionIndexItemEx("n/4483", 0.85)},
         {CompletionIndexItemEx("n/4489", 0.75)},
         {CompletionIndexItemEx("n/66269", 0.6)},
        };

    // Miami, FL
    kv_map_["c/miami,FL,US"] = {
         {CompletionIndexItemEx("n/3585", 0.99)},
         {CompletionIndexItemEx("n/141074", 0.76)},
         {CompletionIndexItemEx("n/146618", 0.6)},
        };

    // Vegas, NV
    kv_map_["c/las vegas,NV,US"] = {
         {CompletionIndexItemEx("n/85108", 0.99)},
         {CompletionIndexItemEx("n/65009", 0.8)},
         {CompletionIndexItemEx("n/7408", 0.5)},
        };
  }

  shared_ptr<CompleteSuggestion> GetDummyNeighborhood(const string& id) {
    const region_data::tNeighborhoodInfoWithBounds* nbr =
        region_data::Neighborhoods::Instance().LookupUniqueByEntityId(id);

    ASSERT_NOTNULL(nbr);

    static int counter = 0;
    shared_ptr<CompleteSuggestion> suggestion(new CompleteSuggestion);
    suggestion->src_type = ::entity::kEntityTypeNeighborhood;
    suggestion->src_id = id;

    suggestion->base_score = max(10.0, (50.0 - (++counter) * 5)) / 100;
    suggestion->freq = 100 - counter;

    suggestion->display = nbr->loc.name;
    suggestion->latitude = nbr->lat;
    suggestion->longitude = nbr->lon;
    suggestion->country = nbr->loc.country_code;
    suggestion->annotations = {nbr->loc.mcd, nbr->loc.state, nbr->loc.country_code};

    suggestion->normalized = ::region_data::utils::NormalizeString(nbr->loc.name);
    return suggestion;
  }

  void DumpIndexFile() {
    LOG(INFO) << "Dumping index.";
    {
      ofstream f(gFlag_output_index_file.c_str());
      serial::Serializer::ToBinary(f, kv_map_);
    }
    {
      const string file = file::ReplaceExtension(gFlag_output_index_file, ".json");
      ofstream f(file.c_str());
      serial::Serializer::ToJSON(f, kv_map_, {1, 1});
    }
  }

  void DumpFalconFile() {
    LOG(INFO) << "Dumping falcon.";
    {
      ofstream f(gFlag_output_falcon_file.c_str());
      serial::Serializer::ToBinary(f, falcon_);
    }
    {
      const string file = file::ReplaceExtension(gFlag_output_falcon_file, ".json");
      ofstream f(file.c_str());
      serial::Serializer::ToJSON(f, falcon_, {1, 1});
    }
  }

  unordered_map<SuggestionId, vector<CompletionIndexItemEx>> kv_map_;
  unordered_map<SuggestionId, shared_ptr<CompleteSuggestion>> falcon_;
};

}  // namespace suggest

int init_main() {
  suggest::DummyNeighborhoods attr;
  attr.Gen();

  return 0;
}
