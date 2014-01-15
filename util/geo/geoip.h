// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

// Uses geo ip libraries' lite database to map IPv4 numbers to countries. This
// is available with a BSD like license in Debian. Free (this) version does not
// have city / state / region etc. granularity, only countries.

#ifndef _GEO_LIB_GEOIP_H_
#define _GEO_LIB_GEOIP_H_

#include <GeoIP.h>
#include <memory>
#include <string>
#include "base/common.h"

extern string gFlag_geoip_database_file;

namespace geoip {

inline shared_ptr<GeoIP> GeoIp() {
  static shared_ptr<GeoIP> gi(GeoIP_open(gFlag_geoip_database_file.c_str(),
                                         GEOIP_STANDARD));
  return gi;
}

// Input must be a v4 IP number. Returns empty string on failure.
// This version is more efficient for IP numbers.
inline string IpToCountryCode(const string& ip) {
  auto ret = GeoIP_country_code_by_addr(GeoIp().get(), ip.c_str());
  return ret != nullptr ? ret : "";
}

// Input must be a v4 IP number or a host (e.g. room77.com). If you want to
// resolve IP numbers to countries exclusively, use IpToCountryCode instead
// (more efficient).
inline string HostToCountryCode(const string& host) {
  auto ret = GeoIP_country_code_by_name(GeoIp().get(), host.c_str());
  return ret != nullptr ? ret : "";
}

}  // namespace geoip

#endif
