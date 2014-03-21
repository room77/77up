// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/log/offline/mock/user_log_element_mock.h"

namespace logging {
namespace test {

tUserLogReadBackElement MakeMockUserLogReadBackElement(const string& category,
                                                       const string& action) {
  tUserLogReadBackElement user_elem;
  user_elem.session_id = "mock_session_id";
  user_elem.received_time = 3;
  user_elem.url = "http://www.room77.com?deb=1";
  user_elem.user_ip = "192.168.77.1";
  user_elem.user_agent = "OpTripBot www.optrip.com";
  user_elem.lang = "en";
  user_elem.host = "www.room77.com";
  user_elem.channel = "web";
  user_elem.is_mobile = false;
  user_elem.user_country = "US";
  user_elem.id = "id";
  user_elem.pid = "pid";
  user_elem.nid = "nid";
  user_elem.category = category;
  user_elem.action = action;
  user_elem.corrected_user_time = 2;
  user_elem.user_time = 1;
  user_elem.cgi_params = {
        {"sgst_id", "c/SF"},
        {"key", "San Francisco"},
        {"lat", "23"},
        {"lon", "24"},
      };

  return user_elem;
}

}  // namespace test
}  // namespace logging

