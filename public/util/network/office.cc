// Copyright 2014 Room 77 Inc.
// Author: sball@room77.com (Stephen Ball)
// Single source to replace hardcoded office ip

#include <arpa/inet.h>

#include "base/common.h"
#include "util/network/office.h"
#include "util/network/dnslookup.h"

FLAG_string(office_hostname, "gate.room77.com",
            "The hostname that maps to the office's IP");
FLAG_int(office_port, 80,
         "The port number used in DNS resolution for finding the office IP");

namespace util {

string Office::ComputeOfficeIP() {
  string addr;
  string office_ip;
  if (!(DNSUtil::Instance().LookupHost(
          gFlag_office_hostname, gFlag_office_port, &addr))) {
    LOG(INFO) << "Unable to resolve office host: " << gFlag_office_hostname;
    office_ip = "-1";
  } else {
    office_ip = inet_ntoa(((const struct sockaddr_in *) addr.c_str())->sin_addr);
  }
  return office_ip;
}

} // namespace util
