// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

#include <iostream>
#include "base/common.h"
#include "util/init/main.h"
#include "geoip.h"

void CheckHost(const string& host, const string& expected_country_code) {
  string country_code = geoip::HostToCountryCode(host);
  // cout << host << "\t" << country_code << endl;
  ASSERT_EQ(country_code, expected_country_code);
}

void CheckIp(const string& ip, const string& expected_country_code) {
  string country_code = geoip::IpToCountryCode(ip);
  // cout << ip << "\t" << country_code << endl;
  ASSERT_EQ(country_code, expected_country_code);
}

int init_main() {
  CheckHost("room77.com", "US");
  CheckHost("google.com", "US");
  CheckHost("baidu.com", "CN");
  CheckHost("louvre.fr", "FR");
  CheckHost("boun.edu.tr", "TR");
  CheckHost("193.140.192.15", "TR");
  CheckHost("", "");
  CheckHost("128.128.128.128", "US");
  CheckIp("193.140.192.15", "TR");
  CheckIp("128.128.128.128", "US");
  CheckIp("", "");
  CheckIp("myip", "");
  return 0;
}
