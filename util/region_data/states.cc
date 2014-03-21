// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

#include <string.h>
#include <algorithm>
#include <unordered_set>

#include "util/region_data/states.h"

#include "util/init/init.h"
#include "util/string/strutil.h"
#include "util/abbreviations/abbreviations.h"
#include "util/region_data/countries.h"
#include "util/region_data/utils.h"

FLAG_string(em_states_id, "em_states",
            "The default entity manager id for neighborhoods.");

FLAG_string(region_states_file,
            "static_data/push/auto/region_data/states/states.csv",
            "List of states.");

namespace region_data {

bool States::Initialize() {
  // Index by name, prefix and latlong.
  super::ConfigParams params;
  params.file = gFlag_region_states_file;
  params.create_entity_ids = true;
  params.build_entity_id_index = false;
  params.build_name_index = true;
  params.name_index_is_unique = false;
  params.build_latlong_index = false;

  ASSERT(super::Initialize(params));

  // Index by codes.
  for (const shared_ptr<tState>&  data : storage()) {
    // ISO 2char.
    string country_code = utils::NormalizeString(data->country_iso_code_2char);
    string code = utils::NormalizeString(data->iso_code_short);
    if (!code.empty()) {
      state_iso_code_index_.AddToIndex(make_pair(code, country_code),
                                       data.get(), true);
      name_index_.AddToIndex(code, data.get(), false);
    }

    // FIPS code.
    code = utils::NormalizeString(data->fips_code_short);
    if (!code.empty()) {
      state_fips_code_index_.AddToIndex(make_pair(code, country_code),
                                       data.get(), true);
    }

    // Name and country code.
    unordered_set<string> alternate_names;
    data->get_alternate_names(&alternate_names);
    alternate_names.insert(data->name);
    for (const string& name : alternate_names) {
      state_name_index_.AddToIndex(make_pair(utils::NormalizeString(name),
                                             country_code), data.get(), true);
    }
  }

  // PQ is alternate for QC.
  const tState* s = LookupByCode("QC", "CA");
  if (s != nullptr) {
    state_iso_code_index_.AddToIndex(make_pair("PQ", "CA"), s, true);
    name_index_.AddToIndex("PQ", s, false);
  }
  return true;
}

const tState* States::LookupByCode(const string& input_code,
                                   const string& input_country_code,
                                   tStateCodeType type) const {
  const tState* s = nullptr;
  const string code = utils::NormalizeString(input_code);
  const string country_code = utils::NormalizeString(input_country_code);

  if (code.empty() || country_code.empty()) return s;

  switch(type) {
    case kStateCodeTypeISO: {
      s = state_iso_code_index_.RetrieveUnique(make_pair(code, country_code));
      break;
    }
    case kStateCodeTypeFips: {
      s = state_fips_code_index_.RetrieveUnique(make_pair(code, country_code));
      if (s == NULL) {
        // Assume the input country code is FIPS and get the equivalent ISO
        // code.
        const tCountry* country =  Countries::Instance().LookupByCode(
            country_code, kCountryCodeTypeFips);
        if (country != NULL && country->iso_code_2char != country_code) {
          s = state_fips_code_index_.RetrieveUnique(
              make_pair(code, country_code));
        }
      }
      break;
    }
    default:
      LOG(INFO) << "Invalid code type: " << type;
      break;
  }
  return s;
}

const tState* States::LookupByCodeMatchAny(const string& code,
    const string& country_code) const {
  const tState* state = LookupByCode(code, country_code,
                                          kStateCodeTypeISO);
  if (state != NULL) return state;

  return LookupByCode(code, country_code, kStateCodeTypeFips);
}

const tState* States::LookupByNameAndCountryCode(const string& input_name,
    const string& input_country_code) const {
  static util::abbr::Abbreviation::shared_proxy abbr =
      util::abbr::Abbreviation::make_shared("name_abbr");
  ASSERT_NOTNULL(abbr);
  string name = utils::NormalizeString(
      abbr->ReplaceAllAbbreviations(input_name));
  const string country_code = utils::NormalizeString(input_country_code);
  return state_name_index_.RetrieveUnique(make_pair(name, country_code));
}

// Returns the complete FIPS code for the state.
string States::GetFIPSCompleteCode(tState* state) {
  if (state->fips_code_short.empty()) return "";

  string country_code = state->country_iso_code_2char;
  const tCountry* country =  Countries::Instance().LookupByCode(
      state->country_iso_code_2char, kCountryCodeTypeISO2Char);
  if (country != NULL && !country->fips_code.empty()) {
    country_code = country->fips_code;
  }
  return GetFIPSCompleteCode(state->fips_code_short, country_code);
}

// Register the states with the enity manager.
auto reg_em_states = ::entity::EntityManager::bind("em_states", "",
    InitializeConfigureConstructor<States, string>());

}  // namespace region_data

// Init before code translator.
INIT_ADD("states", 0, []{ region_data::States::Instance(); });
