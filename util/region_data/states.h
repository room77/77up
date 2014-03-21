// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

// This file defines all the data that is stored for each state.
// A state can be identified using either ISO 3166-1 codes
// (http://en.wikipedia.org/wiki/ISO_3166-1) or FIPS code
// (http://en.wikipedia.org/wiki/List_of_FIPS_state_codes).

#ifndef _UTIL_REGION_DATA_STATES_H_
#define _UTIL_REGION_DATA_STATES_H_

#include <functional>

#include "base/defs.h"
#include "util/string/strutil.h"
#include "util/file/csvreader.h"
#include "util/entity/entity_id.h"
#include "util/i18n/config.h"
#include "util/region_data/region.h"

extern string gFlag_em_states_id;

namespace region_data {

// This struct defines the per state data.
struct tState : public tRegion {
  // Returns the entity type for the given entity.
  virtual entity::EntityType GetEntityType() const { return entity::kEntityTypeState; }

  virtual void SetEntityId() {
    SetEid(::entity::GetEntityIdFromBaseId(entity::kEntityTypeState,
        strutil::JoinTwoStrings(country_iso_code_2char, iso_code_short, "-")));
  }

  // The short iso state code. For example, for the state 'California' the
  // short code would be 'CA'. The complete ISO code can be generated as
  // '<country_iso_code_2char>-<iso_code_short>'.
  string iso_code_short;

  // The short FIPS code. For example, for the state 'California' the
  // fips code would be '06'. The complete FIPS code can be generated as
  // '<country_fips_code><fips_code_short>'
  string fips_code_short;

  // The 2 char iso code for the country.
  string country_iso_code_2char;

  CSV(iso_code_short | fips_code_short | country_iso_code_2char | name |
      alternate_names);

  // This is only for testing. This struct should never be serialized in
  // production code. Use tStateReply instead.
  SERIALIZE(eid*8 / iso_code_short*1 / fips_code_short*2 /
            country_iso_code_2char*3 / name*4 / alternate_names*5 / lat*6 /
            lon*7);
};

// The state data returned as RPC Reply.
struct tStateReply : public tRegionReply {
  string state_code;

  // The country info.
  string country_code;  // This is the ISO 2 char Code.

  SERIALIZE(eid*4 / name*1 / state_code*2 / country_code*3);
};

// Returns the state RPC reply after converting the state storage data.
struct state_storage_to_reply :
    public region_storage_to_reply<tState, tStateReply> {
  typedef region_storage_to_reply<tState, tStateReply> super;

  virtual tStateReply operator() (const tState& s) const {
    tStateReply reply = super::operator()(s);
    reply.country_code = s.country_iso_code_2char;
    return reply;
  }

  virtual tStateReply operator() (const tState* s) const {
    return operator()(*s);
  }
};

// The state code to lookup for indexing.
enum tStateCodeType {
  kStateCodeTypeISO,
  kStateCodeTypeFips,
};

struct order_states_by_name : binary_function <tState, tState, bool> {
  // First sort by pre-defined state priority list, then sort by name.
  bool operator() (const tState& s1, const tState& s2) const {
    int p1 = I18N::ListPriority(s1.country_iso_code_2char);
    int p2 = I18N::ListPriority(s2.country_iso_code_2char);

    // The higher the priority, the lower the number.
    if (p1 != p2) return (p1 < p2);

    // Sort names by alphabetical order.
    if (s1.name != s2.name) return (s1.name < s2.name);


    // Sort country codes in alphabetical order.
    if (s1.country_iso_code_2char != s2.country_iso_code_2char)
      return (s1.country_iso_code_2char < s2.country_iso_code_2char);

    // Compare by iso codes.
    return (s1.iso_code_short < s2.iso_code_short);
  }

  bool operator() (const tState* s1, const tState* s2) const {
    return operator ()(*s1, *s2);
  }
};

// This class provides an interface to access the states data.
// Since this data is read only, there is only a single instance of the data.
class States : public Region<tState,  order_states_by_name> {
  typedef Region<tState, order_states_by_name> super;

 public:
  virtual ~States() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize();

  static const States& Instance() {  // singleton instance
    struct Creator {
      States* CreateStates() {
        LOG(INFO) << "Creating States using Id: " << gFlag_em_states_id;
        mutable_shared_proxy proxy = make_shared(gFlag_em_states_id);
        ASSERT_NOTNULL(proxy);
        pin(proxy);
        return dynamic_cast<States*>(proxy.get());
      }
    };
    static States* the_one = Creator().CreateStates();
    return *the_one;
  }

  // Returns the state that matches the entity id.
  virtual const tState* LookupUniqueByEntityId(const string& entity_id) const {
    const string base_id = ::entity::GetBaseIdFromEntityId<string>(entity_id);
    vector<string> parts;
    if (strutil::SplitStringNParts(base_id, "-", &parts, 1) != 2)
      return nullptr;
    return LookupByCode(parts[1], parts[0]);
  }

  // Returns the state that matches the given state code for the type specified.
  // Note that the country code should preferably be the ISO code for all
  // state code types. However, if the FIPS lookup fails with ISO, the input
  // country code is converted to ISO and another lookup done.
  const tState* LookupByCode(const string& code, const string& country_code,
      tStateCodeType type = kStateCodeTypeISO) const;

  // Returns the state that matches the given state code and the ISO country
  // code for any of the types.
  // The lookup order for char codes is: 'iso', 'fips'.
  const tState* LookupByCodeMatchAny(const string& code,
                                     const string& country_code) const;

  // Returns the state matching the given name. We expect the country code to
  // be in ISO format.
  const tState* LookupByNameAndCountryCode(const string& name,
                                           const string& country_code) const;

  // Returns the state name for the given state code.
  string GetStateName(const string& code, const string& country_code,
      tStateCodeType type = kStateCodeTypeISO) const {
   const tState* state = LookupByCode(code, country_code, type);
   if (state) return state->name;
   return "";
  }

  // Returns the complete ISO code for the state.
  static string GetISOCompleteCode(tState* state) {
    return GetISOCompleteCode(state->iso_code_short,
                              state->country_iso_code_2char);
  }

  // Returns the complete FIPS code for the state.
  static string GetFIPSCompleteCode(tState* state);

  // Returns the complete ISO code for the state code and the country.
  static string GetISOCompleteCode(const string& code, const string& country_code) {
    if (code.empty()) return "";
    return country_code + "-" + code;
  }

  // Returns the complete FIPS code for the state code and the country.
  static string GetFIPSCompleteCode(const string& code, const string& country_code) {
    if (code.empty()) return "";
    return country_code + code;
  }

  // Returns the ISO short code for the complete state code and the country.
  // If the country code is not given, everything before the '-' is removed.
  static string GetISOShortCode(const string& code, const string& country_code = "") {
    if (code.empty()) return "";

    // This is the prefix to removed to get the short code.
    string prefix_to_remove = country_code + "-";
    size_t pos = code.find(prefix_to_remove);
    if (pos != string::npos)
      return code.substr(pos + prefix_to_remove.size());

    // By default we simply return the input code back.
    return code;
  }

  // Returns the FIPS short code for the complete state code and the country.
  // If the country code is not given, everything before the first numeric
  // character is removed.
  static string GetFIPSShortCode(const string& code, const string& country_code = "") {
    if (code.empty()) return "";

    if (!country_code.empty()) {
      size_t pos = code.find(country_code);
      if (pos != string::npos)
        return code.substr(pos + country_code.size());
    } else {
      size_t pos = code.find_first_of("0123456789");
      if (pos != string::npos)
        return code.substr(pos);
    }

    // By default we simply return the input code back.
    return code;
  }

 protected:
  // Declare this class as friend so that it call its constructor.
  friend class InitializeConfigureConstructor<States, string>;

  States() {}

 private:
  // Index states by iso short code.
  // The key is <iso state code, iso country code>.
  Index<pair<const char*, const char*>, const tState*> state_iso_code_index_;
  // Index states by fips short code.
  // The key is <fips state code, *iso* country code>.
  Index<pair<const char*, const char*>, const tState*> state_fips_code_index_;
  // Index states by name.
  // The key is <iso state name, iso country code>.
  Index<pair<const char*, const char*>, const tState*> state_name_index_;
};

}  // namespace region_data

#endif  // _UTIL_REGION_DATA_STATES_H_
