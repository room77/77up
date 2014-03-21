// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _GEO_UTIL_DOMAIN_BOOST_DOMAIN_BOOST_H_
#define _GEO_UTIL_DOMAIN_BOOST_DOMAIN_BOOST_H_

#include "base/common.h"

namespace geo {
namespace domain_boost {

class DomainBoost {
 public:
  static DomainBoost& Instance() {  // singleton instance
   static DomainBoost the_one;
   return the_one;
  }

  // Returns the boost a destination country should get given a caller country.
  // TODO(pramodg): Use logs to compute this 2-D array.
  double Boost(const string& caller_country, const string& destination_country) {
    if (caller_country == destination_country) return 3;
    return 1;
  }

 protected:
  DomainBoost() {
    Initialize();
  }

  void Initialize() {}
};

}  // namespace domain_boost
}  // namespace geo


#endif  // _GEO_UTIL_DOMAIN_BOOST_DOMAIN_BOOST_H_
