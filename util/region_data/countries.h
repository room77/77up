// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

// This file defines all the data that is stored for each country.
// A country can be identified using either ISO 3166-1 codes
// (http://en.wikipedia.org/wiki/ISO_3166-1) or FIPS code
// (http://en.wikipedia.org/wiki/List_of_FIPS_country_codes).

#ifndef _UTIL_REGION_DATA_COUNTRIES_H_
#define _UTIL_REGION_DATA_COUNTRIES_H_

#include <functional>

#include "base/defs.h"
#include "util/string/strutil.h"
#include "util/file/csvreader.h"
#include "util/entity/entity_id.h"
#include "util/i18n/config.h"
#include "util/region_data/region.h"

extern string gFlag_em_countries_id;

namespace region_data {

// This struct defines the per country data.
struct tCountry : public tRegion {
  // Returns the entity type for the given entity.
  virtual entity::EntityType GetEntityType() const { return entity::kEntityTypeCountry; }

  virtual void SetEntityId() {
    SetEid(::entity::GetEntityIdFromBaseId(entity::kEntityTypeCountry,
                                           iso_code_2char));
  }

  // The 2 char iso country code.
  string iso_code_2char;

  // The 3 char iso country code.
  string iso_code_3char;

  // The numeric iso country code.
  int iso_code_numeric = 0;

  // The FIPS code. This is usually the same as ISO, however sometimes the
  // two don't match. For example, the ISO code for Turkey is TR but the FIPS
  // code is TU.
  string fips_code;

  // The IATA country code.
  string iata_code;

  // The ISO currency code (http://en.wikipedia.org/wiki/ISO_4217).
  string iso_currency_code;

  CSV(iso_code_2char | iso_code_3char | iso_code_numeric | fips_code |
      iata_code | name | iso_currency_code | alternate_names |
      lat | lon);

  // This is only for testing. This struct should never be serialized in
  // production code. Use tCountryReply instead.
  SERIALIZE(eid*11 / iso_code_2char*1 / iso_code_3char*2 / iso_code_numeric*3 /
            fips_code*4 / iata_code*5 / name*6 / iso_currency_code*7 /
            alternate_names*8 / lat*9 / lon*10);
};

// The country data returned as RPC Reply.
struct tCountryReply : public tRegionReply {
  // The country info.
  string country_code;  // This is the ISO 2 char Code.

  SERIALIZE(eid*5 / name*1 / latitude*2 / longitude*3 / country_code*4);
};

// Returns the country RPC reply after converting the country storage data.
struct country_storage_to_reply :
    public region_storage_to_reply<tCountry, tCountryReply> {
  typedef region_storage_to_reply<tCountry, tCountryReply> super;

  virtual tCountryReply operator() (const tCountry& c) const {
    tCountryReply reply = super::operator()(c);
    reply.country_code = c.iso_code_2char;
    return reply;
  }

  virtual tCountryReply operator() (const tCountry* c) const {
    return operator()(*c);
  }
};

// The different country code types.
enum tCountryCodeType {
  kCountryCodeTypeISO2Char,
  kCountryCodeTypeISO3Char,
  kCountryCodeTypeFips,
};

struct order_countries_by_name : binary_function <tCountry, tCountry, bool> {
  // First sort by pre-defined country priority list, then sort by name.
  bool operator() (const tCountry& c1, const tCountry& c2) const {
    int p1 = I18N::ListPriority(c1.iso_code_2char);
    int p2 = I18N::ListPriority(c2.iso_code_2char);
    return (p1 < p2 || (p1 == p2 && c1.name < c2.name));
  }

  bool operator() (const tCountry* c1, const tCountry* c2) const {
    return operator ()(*c1, *c2);
  }
};

// Matches the country code. Takes into account some special cases.
struct country_code_matcher : binary_function <string, string, bool> {
  bool operator() (const string& left, const string& right) const {
    string upper_left = strutil::ToUpper(left);
    string upper_right = strutil::ToUpper(right);
    if (upper_left == upper_right) return true;

    if (upper_left == "CN") return CheckChinaCodes(upper_right);
    if (upper_right == "CN") return CheckChinaCodes(upper_left);

    if (upper_left == "US") return CheckUSCodes(upper_right);
    if (upper_right == "US") return CheckUSCodes(upper_left);

    return false;
  }

 private:
  // This is for backward compatibility with ISO 3166-1 where Hong Kong, Taiwan, and Macau have
  // their own country codes in ISO 3166-1 but were added as subdivisions of China in ISO 3166-2.
  // http://en.wikipedia.org/wiki/ISO_3166-2:CN
  bool CheckChinaCodes(const string& other) const {
    return (other == "HK" || other == "TW" || other == "MO");
  }

  // This is for backward compatibility with ISO 3166-1 where 6 countries have
  // their own country codes in ISO 3166-1 but were added as subdivisions of US in ISO 3166-2.
  // http://en.wikipedia.org/wiki/ISO_3166-2:US
  bool CheckUSCodes(const string& other) const {
    return (other == "USA" || other == "VI" || other == "GU" || other == "PR" || other == "MP"
        || other == "AS" || other == "UM");
  }
};

// This class provides an interface to access the countries data.
// Since this data is read only, there is only a single instance of the data.
class Countries : public Region<tCountry, order_countries_by_name> {
  typedef Region<tCountry, order_countries_by_name> super;
  using Comparator = super::CompType;

 public:
  virtual  ~Countries() {}

  // Initialize the class.
  virtual bool Initialize();

  static const Countries& Instance() {  // singleton instance
    struct Creator {
      Countries* CreateCountries() {
        LOG(INFO) << "Creating Countries using Id: " << gFlag_em_countries_id;
        mutable_shared_proxy proxy = make_shared(gFlag_em_countries_id);
        ASSERT_NOTNULL(proxy);
        pin(proxy);
        return dynamic_cast<Countries*>(proxy.get());
      }
    };
    static Countries* the_one = Creator().CreateCountries();
    return *the_one;
  }

  // Returns the country that matches the entity id.
  virtual const tCountry* LookupUniqueByEntityId(const string& entity_id) const {
    const string code = ::entity::GetBaseIdFromEntityId<string>(entity_id);
    return LookupByCode(code);
  }

  // Generates a list of regions matching the given name.
  // The return value is the size of the matched regions.
  virtual int LookupByName(const string& name,
                           vector<const tCountry*>* result,
                           int max_results = std::numeric_limits<int>::max(),
                           const Comparator& comp = Comparator()) const;

  // Returns the country that matches the given country code for the type
  // specified.
  const tCountry* LookupByCode(const string& code,
      tCountryCodeType type = kCountryCodeTypeISO2Char) const;

  // Returns the country that matches the given country code for any of the
  // types. The lookup order for char codes is: 'iso_2_char', 'fips'.
  // If the code has 3 chars it is checked against 'iso_3_char'.
  const tCountry* LookupByCodeMatchAny(const string& code) const;

  // Returns the currency code for the given country code.
  string GetCurrencyCode(const string& code,
      tCountryCodeType type = kCountryCodeTypeISO2Char) const {
    const tCountry* c = LookupByCode(code, type);
    if (c) return c->iso_currency_code;
    return "";
  }

  // Returns the country name for the given country code.
  string GetCountryName(const string& code,
      tCountryCodeType type = kCountryCodeTypeISO2Char) const {
   const tCountry* c = LookupByCode(code, type);
   if (c) return c->name;
   return "";
  }

  // Returns the updated code in ISO 3066-2 for some of the country codes in ISO 3066-1.
  static string GetUpdateCode(const string& code);

 protected:
  // Declare this class as friend so that it call its constructor.
  friend class InitializeConfigureConstructor<Countries, string>;

  Countries() {}

 private:
  // Index countries by 2 char iso code.
  Index<const char*, const tCountry*> country_iso_code_2char_index_;
  // Index countries by 3 char iso code.
  Index<const char*, const tCountry*> country_iso_code_3char_index_;
  // Index countries by fips code.
  Index<const char*, const tCountry*> country_fips_code_index_;
};

}  // namespace region_data

#endif  // _UTIL_REGION_DATA_COUNTRIES_H_
