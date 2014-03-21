#ifndef _UTIL_I18N_CONFIG_H_
#define _UTIL_I18N_CONFIG_H_

#include "util/string/strutil.h"

namespace I18N {
  //
  // these functions specify what country codes are currently supported
  //

  inline bool SupportedCountryCode(const string& code) {
    return true;  // all countries are supported
    // return (code == "US" || code == "CA");
  }
  /*
  inline void GetSupportedCountryCodes(unordered_set<string> *list) {
    list->clear();
    list->insert("US");
    list->insert("CA");
  }
  */
  // give these countries priority when sorting by name
  inline int ListPriority(const string& code) {
    if (code == "US")
      return 0;
    else if (code == "CA")
      return 1;
    else
      return 2;
  }

  // standard "city, state zip" formatting
  inline string StandardLocation(const string& city, const string& state,
                                 const string& postal_code,
                                 const string& country) {
    string s = strutil::Capitalize(city);
    if (!state.empty()) {
      s += ", ";
      s += state;
    }
    if (!postal_code.empty()) {
      s += " ";
      s += postal_code;
    }
    if (!country.empty()) {
      s += " [";
      s += country;
      s += "]";
    }
    return s;
  }
}


#endif
