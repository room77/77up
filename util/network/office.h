// Copyright 2014 Room 77 Inc.
// Author: sball@room77.com (Stephen Ball)
// Single source to replace hardcoded office ip

#ifndef _PUBLIC_UTIL_OFFICE_H
#define _PUBLIC_UTIL_OFFICE_H

#include "base/common.h"

namespace util {

class Office {
 public:

  static const string& GetOfficeIP() {
    const static string office_ip = ComputeOfficeIP();
    return office_ip;
  }

  static bool IsOfficeIP(const string& ip) {
    return (ip == GetOfficeIP() || ip == "127.0.0.1" || ip == "127.0.1.1" ||
            ip.find("192.168") == 0);
  }

 private:
  static string ComputeOfficeIP();

};

} // namespace util

#endif // _UTIL_OFFICE_H
