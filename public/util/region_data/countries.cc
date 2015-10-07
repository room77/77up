// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

#include <string.h>
#include <algorithm>
#include <unordered_set>

#include "util/region_data/countries.h"

#include "util/init/init.h"
#include "util/string/strutil.h"
#include "util/abbreviations/abbreviations.h"
#include "util/region_data/utils.h"
#include "util/templates/container_util.h"

FLAG_string(em_countries_id, "em_countries",
            "The default entity manager id for countries.");

FLAG_string(region_countries_file,
            "static_data/push/auto/region_data/countries/countries.csv",
            "List of countries.");

namespace region_data {

bool Countries::Initialize() {
  // Index by name, prefix and latlong.
  super::ConfigParams params;
  params.file = gFlag_region_countries_file;
  params.create_entity_ids = true;
  params.build_entity_id_index = false;
  params.build_name_index = true;
  params.name_index_is_unique = true;
  params.build_latlong_index = true;
  params.lat_grid_size = 30000000;
  params.long_grid_size = 40000000;

  ASSERT(super::Initialize(params));

  // Index by codes.
  for (const shared_ptr<tCountry>& data : storage()) {
    // ISO 2char.
    string code = utils::NormalizeString(data->iso_code_2char);
    if (!code.empty()) {
      country_iso_code_2char_index_.AddToIndex(code, data.get(), true);
    }

    // ISO 3char.
    code = utils::NormalizeString(data->iso_code_3char);
    if (!code.empty()) {
      country_iso_code_3char_index_.AddToIndex(code, data.get(), true);
    }

    // FIPS code.
    code = utils::NormalizeString(data->fips_code);
    if (!code.empty()) {
      country_fips_code_index_.AddToIndex(code, data.get(), true);
    }
  }
  return true;
}

const tCountry* Countries::LookupByCode(const string& input_code,
                                        tCountryCodeType type) const {
  const tCountry* c = NULL;
  const string code = utils::NormalizeString(input_code);
  switch(type) {
    case kCountryCodeTypeISO2Char: {
      c = country_iso_code_2char_index_.RetrieveUnique(code);
      break;
    }
    case kCountryCodeTypeISO3Char: {
      c = country_iso_code_3char_index_.RetrieveUnique(code);
      break;
    }
    case kCountryCodeTypeFips: {
      c = country_fips_code_index_.RetrieveUnique(code);
      break;
    }
    default:
      LOG(INFO) << "Invalid code type: " << type;
      break;
  }
  return c;
}

const tCountry* Countries::LookupByCodeMatchAny(
    const string& input_code) const {
  const string code = utils::NormalizeString(input_code);

  // If the code has 3 chars, it can only be in iso_3_char.
  if (code.length() == 3)
    return LookupByCode(code, kCountryCodeTypeISO3Char);

  const tCountry* c = LookupByCode(code, kCountryCodeTypeISO2Char);
  if (c != NULL) return c;

  return LookupByCode(code, kCountryCodeTypeFips);
}

// Generates a list of regions matching the given name.
// The return value is the size of the matched regions.
int Countries::LookupByName(const string& name, vector<const tCountry*>* result,
    int max_results, const Comparator& comp) const {
  super::LookupByName(name, result, max_results, comp);
  if (result->empty()) {
    const tCountry* c = LookupByCodeMatchAny(name);
    if (c != nullptr) result->push_back(c);
  }
  return result->size();
}

// Returns the updated code in ISO 3066-2 for some of the country codes in ISO 3066-1.
string Countries::GetUpdateCode(const string& code) {
  static const unordered_map<string, string> kUpdatedCodes = {
    {"HV", "BF"}, {"DY", "BJ"}, {"ZR", "CD"}, {"BU", "MM"}, {"TP", "TL"},
    {"NH", "VU"}, {"RH", "ZW"}
  };

  return ::util::tl::FindWithDefault(kUpdatedCodes, code, code);
}

// Register the countries with the enity manager.
auto reg_em_countries = ::entity::EntityManager::bind("em_countries", "",
    InitializeConfigureConstructor<Countries, string>());

}  // namespace region_data

// Init before code translator.
INIT_ADD("countries", 0, []{ region_data::Countries::Instance(); });
