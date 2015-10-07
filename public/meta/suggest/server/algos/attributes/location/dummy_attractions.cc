// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "util/file/file.h"
#include "base/defs.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/region_data/attractions.h"
#include "util/region_data/utils.h"
#include "util/serial/serializer.h"

FLAG_string(output_falcon_file,
            "/home/share/data/suggest/attribute/test/attractions/current/attractions_falcon.dat",
            "The falcon file to be generated .");

FLAG_string(output_index_file,
            "/home/share/data/suggest/attribute/test/attractions/current/attractions_index.dat",
            "The index file to be generated.");

namespace suggest {

class DummyAttractions {
 public:
  void Gen() {
    PopulateFalcon();
    PopulateIndex();
    DumpIndexFile();
    DumpFalconFile();
  }

 private:
  void PopulateFalcon() {
    // San Francisco, CA
    falcon_["a/LombardStr,SF,ca,us"] = GetDummyAttraction("a/LombardStr,SF,ca,us");  // Lombard street
    falcon_["a/UnionSqu,SF,ca,us"] = GetDummyAttraction("a/UnionSqu,SF,ca,us");  // Union square
    falcon_["a/FishermansWha,SF,ca,us"] = GetDummyAttraction("a/FishermansWha,SF,ca,us");  // Fisherman's Wharf
    falcon_["a/Pier39,SF,ca,us"] = GetDummyAttraction("a/Pier39,SF,ca,us");  // Pier 39
    falcon_["a/PalaceofFinArt,SF,ca,us"] = GetDummyAttraction("a/PalaceofFinArt,SF,ca,us");  // Palace of Fine Arts

    // Boston, MA
    falcon_["a/HynesConCen,B,ma,us"] = GetDummyAttraction("a/HynesConCen,B,ma,us");  // Hynes Convention Center
    falcon_["a/BostonCom,B,ma,us"] = GetDummyAttraction("a/BostonCom,B,ma,us");  // Boston Common
    falcon_["a/CopleyPla,B,ma,us"] = GetDummyAttraction("a/CopleyPla,B,ma,us");  // Copley Place
    falcon_["a/BostonPubGar,B,ma,us"] = GetDummyAttraction("a/BostonPubGar,B,ma,us");  // Boston Public Garden
    falcon_["a/FenwayPar,B,ma,us"] = GetDummyAttraction("a/FenwayPar,B,ma,us");  // Fenway Park

    // New York, NY
    falcon_["a/TimesSqu,NY,ny,us"] = GetDummyAttraction("a/TimesSqu,NY,ny,us");  // Times Square
    falcon_["a/Broadway,NY,ny,us"] = GetDummyAttraction("a/Broadway,NY,ny,us");  // Broadway
    falcon_["a/EmpireStaBui,NY,ny,us"] = GetDummyAttraction("a/EmpireStaBui,NY,ny,us");  // Empire State Building
    falcon_["a/GrandCenTer,NY,ny,us"] = GetDummyAttraction("a/GrandCenTer,NY,ny,us");  // Grand Central Terminal
    falcon_["a/RockefellerCen,NY,ny,us"] = GetDummyAttraction("a/RockefellerCen,NY,ny,us");  // Rockefeller Center

    // Atlanta, GA
    falcon_["a/CentennialOlyPar,A,ga,us"] = GetDummyAttraction("a/CentennialOlyPar,A,ga,us");  // Centennial Olympic Park
    falcon_["a/CobbGalCen,A,ga,us"] = GetDummyAttraction("a/CobbGalCen,A,ga,us");  // Cobb Galleria Centre
    falcon_["a/GeorgiaGovsMan,A,ga,us"] = GetDummyAttraction("a/GeorgiaGovsMan,A,ga,us");  // Georgia Governor's Mansion
    falcon_["a/LegolandDisCen,A,ga,us"] = GetDummyAttraction("a/LegolandDisCen,A,ga,us");  // Legoland Discovery Center
    falcon_["a/SwanHou,A,ga,us"] = GetDummyAttraction("a/SwanHou,A,ga,us");  // Swan House

    // Chicago, IL
    falcon_["a/MillenniumPar,C,il,us"] = GetDummyAttraction("a/MillenniumPar,C,il,us");  // Millennium Park
    falcon_["a/SkydeckLed,C,il,us"] = GetDummyAttraction("a/SkydeckLed,C,il,us");  // Skydeck Ledge
    falcon_["a/WillisTow,C,il,us"] = GetDummyAttraction("a/WillisTow,C,il,us");  // Willis Tower
    falcon_["a/HancockTow,C,il,us"] = GetDummyAttraction("a/HancockTow,C,il,us");  // Hancock Tower
    falcon_["a/AllstateAre,C,il,us"] = GetDummyAttraction("a/AllstateAre,C,il,us");  // Allstate Arena

    // London, GB
    falcon_["a/bigben,L,eng,gb"] = GetDummyAttraction("a/bigben,L,eng,gb");  // Big Ben
    falcon_["a/TrafalgarSqu,L,eng,gb"] = GetDummyAttraction("a/TrafalgarSqu,L,eng,gb");  // Trafalgar Square
    falcon_["a/RoyalAlbHal,L,eng,gb"] = GetDummyAttraction("a/RoyalAlbHal,L,eng,gb");  // Royal Albert Hall
    falcon_["a/BuckinghamPal,L,eng,gb"] = GetDummyAttraction("a/BuckinghamPal,L,eng,gb");  // Buckingham Palace
    falcon_["a/HydePar,L,eng,gb"] = GetDummyAttraction("a/HydePar,L,eng,gb");  // Hyde Park

    // Paris, FR
    falcon_["a/arcdeTriomphe,P,34,fr"] = GetDummyAttraction("a/arcdeTriomphe,P,34,fr");  // Arc de Triomphe
    falcon_["a/EiffelTow,P,34,fr"] = GetDummyAttraction("a/EiffelTow,P,34,fr");  // Eiffel Tower
    falcon_["a/LouvreMus,P,34,fr"] = GetDummyAttraction("a/LouvreMus,P,34,fr");  // Louvre Museum
    falcon_["a/ParisOpe,P,34,fr"] = GetDummyAttraction("a/ParisOpe,P,34,fr");  // Paris Opera
    falcon_["a/DisneylandPar,P,34,fr"] = GetDummyAttraction("a/DisneylandPar,P,34,fr");  // Disneyland Paris
  }

  void PopulateIndex() {
    // San Francisco, CA
    kv_map_["c/san francisco,CA,US"] = {
         {CompletionIndexItemEx("a/LombardStr,SF,ca,us", 0.5)},
         {CompletionIndexItemEx("a/UnionSqu,SF,ca,us", 0.4)},
         {CompletionIndexItemEx("a/FishermansWha,SF,ca,us", 0.3)},
         {CompletionIndexItemEx("a/Pier39,SF,ca,us", 0.2)},
         {CompletionIndexItemEx("a/PalaceofFinArt,SF,ca,us", 0.1)},
        };


    // Boston, MA
    kv_map_["c/boston,MA,US"] = {
         {CompletionIndexItemEx("a/HynesConCen,B,ma,us", 0.5)},
         {CompletionIndexItemEx("a/BostonCom,B,ma,us", 0.4)},
         {CompletionIndexItemEx("a/CopleyPla,B,ma,us", 0.3)},
         {CompletionIndexItemEx("a/BostonPubGar,B,ma,us", 0.2)},
         {CompletionIndexItemEx("a/FenwayPar,B,ma,us", 0.1)},
        };

    // New York, NY
    kv_map_["c/new york,NY,US"] = {
         {CompletionIndexItemEx("a/TimesSqu,NY,ny,us", 0.5)},
         {CompletionIndexItemEx("a/Broadway,NY,ny,us", 0.4)},
         {CompletionIndexItemEx("a/EmpireStaBui,NY,ny,us", 0.3)},
         {CompletionIndexItemEx("a/GrandCenTer,NY,ny,us", 0.2)},
         {CompletionIndexItemEx("a/RockefellerCen,NY,ny,us", 0.1)},
        };

    // Atlanta, GA
    kv_map_["c/atlanta,GA,US"] = {
         {CompletionIndexItemEx("a/CentennialOlyPar,A,ga,us", 0.5)},
         {CompletionIndexItemEx("a/CobbGalCen,A,ga,us", 0.4)},
         {CompletionIndexItemEx("a/GeorgiaGovsMan,A,ga,us", 0.3)},
         {CompletionIndexItemEx("a/LegolandDisCen,A,ga,us", 0.2)},
         {CompletionIndexItemEx("a/SwanHou,A,ga,us", 0.1)},
        };

    // Chicago, IL
    kv_map_["c/chicago,IL,US"] = {
         {CompletionIndexItemEx("a/MillenniumPar,C,il,us", 0.5)},
         {CompletionIndexItemEx("a/SkydeckLed,C,il,us", 0.4)},
         {CompletionIndexItemEx("a/WillisTow,C,il,us", 0.3)},
         {CompletionIndexItemEx("a/HancockTow,C,il,us", 0.2)},
         {CompletionIndexItemEx("a/AllstateAre,C,il,us", 0.1)},
        };

    // London, GB
    kv_map_["c/london,BEN,GB"] = {
         {CompletionIndexItemEx("a/bigben,L,eng,gb", 0.5)},
         {CompletionIndexItemEx("a/TrafalgarSqu,L,eng,gb", 0.4)},
         {CompletionIndexItemEx("a/RoyalAlbHal,L,eng,gb", 0.3)},
         {CompletionIndexItemEx("a/BuckinghamPal,L,eng,gb", 0.2)},
         {CompletionIndexItemEx("a/HydePar,L,eng,gb", 0.1)},
        };

    // Paris, FR
    kv_map_["c/paris,34,FR"] = {
         {CompletionIndexItemEx("a/arcdeTriomphe,P,34,fr", 0.5)},
         {CompletionIndexItemEx("a/EiffelTow,P,34,fr", 0.4)},
         {CompletionIndexItemEx("a/LouvreMus,P,34,fr", 0.3)},
         {CompletionIndexItemEx("a/ParisOpe,P,34,fr", 0.2)},
         {CompletionIndexItemEx("a/DisneylandPar,P,34,fr", 0.1)},
        };
  }

  shared_ptr<CompleteSuggestion> GetDummyAttraction(const string& id) {
    const region_data::tAttraction* attraction =
        region_data::Attractions::Instance().LookupUniqueByEntityId(id);

    ASSERT_NOTNULL(attraction);

    static int counter = 0;
    shared_ptr<CompleteSuggestion> suggestion(new CompleteSuggestion);
    suggestion->src_type = ::entity::kEntityTypeAttraction;
    suggestion->src_id = id;

    suggestion->base_score = max(10.0, (50.0 - (++counter) * 5)) / 100;
    suggestion->freq = 100 - counter;

    suggestion->display = attraction->name;
    suggestion->latitude = attraction->lat;
    suggestion->longitude = attraction->lon;
    suggestion->country = attraction->country_iso_code_2char;
    suggestion->annotations = {attraction->city, attraction->state_iso_code_short,
                               attraction->country_iso_code_2char};

    suggestion->normalized = ::region_data::utils::NormalizeString(attraction->name);
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
  suggest::DummyAttractions attr;
  attr.Gen();

  return 0;
}
