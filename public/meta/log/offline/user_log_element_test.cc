// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/log/offline/user_log_element.h"

#include "test/cc/test_main.h"

namespace logging {
namespace test {

TEST(tUserLogReadBackElement, url) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{referer_url: r.com }");
    EXPECT_EQ("r.com", elem.url);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{url: u.com, referer_url: r.com }");
    EXPECT_EQ("u.com", elem.url);
  }
}

TEST(tUserLogReadBackElement, corrected_user_time) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{server_time: 12 }");
    EXPECT_EQ(12, elem.corrected_user_time);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{corrected_user_time: 13, server_time: 12 }");
    EXPECT_EQ(13, elem.corrected_user_time);
  }
}

TEST(tUserLogReadBackElement, received_time) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{created: 12 }");
    EXPECT_EQ(12, elem.received_time);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{received_time: 13, created: 12 }");
    EXPECT_EQ(13, elem.received_time);
  }
}

TEST(tUserLogReadBackElement, user_agent) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{agent: abc }");
    EXPECT_EQ("abc", elem.user_agent);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{user_agent: pqr, agent: abc }");
    EXPECT_EQ("pqr", elem.user_agent);
  }
}

TEST(tUserLogReadBackElement, user_ip) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{ip: 1.2.3.4 }");
    EXPECT_EQ("1.2.3.4", elem.user_ip);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{user_ip: 2.3.4.5, ip: 1.2.3.4 }");
    EXPECT_EQ("2.3.4.5", elem.user_ip);
  }
}

TEST(tUserLogReadBackElement, user_country) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{user_ip: 50.76.63.141 }");
    EXPECT_EQ("US", elem.user_country);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{user_ip: 193.140.192.15 }");
    EXPECT_EQ("TR", elem.user_country);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{agent: abc }");  // NO IP info.
    EXPECT_EQ("US", elem.user_country);
  }
}

TEST(tUserLogReadBackElement, channel) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{channel: web }");
    EXPECT_EQ("web", elem.channel);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{agent: abc }");  // NO channel info.
    EXPECT_EQ("web", elem.channel);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{channel: desktop }");
    EXPECT_EQ("web", elem.channel);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{channel: ios }");
    EXPECT_EQ("mobile-app-ios", elem.channel);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{channel: android }");
    EXPECT_EQ("mobile-app-android", elem.channel);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{channel: windows }");
    EXPECT_EQ("mobile-app-windows", elem.channel);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{channel: mweb }");
    EXPECT_EQ("mobile-web", elem.channel);
  }
}

TEST(tUserLogReadBackElement, CGI) {
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{url: url.com?p1=x1&p2=x2, cgi_params : {p1 : v1, p2 : v2}}");
    EXPECT_EQ("v1", elem.cgi_params["p1"]);
    EXPECT_EQ("v2", elem.cgi_params["p2"]);
  }
  {
    tUserLogReadBackElement elem;
    elem.FromJSON("{url: url.com?p1=x1&p2=x2, cgi_params : {}}");
    EXPECT_EQ("x1", elem.cgi_params["p1"]);
    EXPECT_EQ("x2", elem.cgi_params["p2"]);
  }
}

}  // namespace test
}  // namespace logging
