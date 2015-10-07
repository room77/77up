// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/log/offline/user_log_element.h"

#include <unordered_map>

#include "util/log/channel/channel.h"
#include "util/geo/geoip.h"
#include "util/network/util.h"
#include "util/templates/container_util.h"

namespace logging {

bool tUserLogReadBackElement::DeserializationCallback() {
  // The following values were changed in JUL 2013. They should be safe to remove after JUL 2014.
  // Fix referer url.
  if (url.empty()) url = referer_url;

  // The following values were changed in SEP 2013. They should be safe to remove after SEP 2014.

  // Fix timestamps.
  if (corrected_user_time == 0) corrected_user_time = server_time;
  if (received_time == 0) received_time = created;

  // Fix user agent.
  if (user_agent.empty()) user_agent = agent;

  // Fix user ip.
  if (user_ip.empty()) user_ip = ip;

  // Fix user country.
  if (user_country.empty()) {
    user_country = geoip::IpToCountryCode(user_ip);
    if (user_country.empty()) user_country = "US";
  }

  // Mapping from old channel types to new channel types.
  // The new channel definitions can be found in meta/util/channel/channel.cc
  static const unordered_map<string, string> channel_mapping = {
        {"desktop", "web"},
        {"ios", "mobile-app-ios"},
        {"android", "mobile-app-android"},
        {"windows", "mobile-app-windows"},
        {"mweb", "mobile-web"},
      };

  if (channel.size()) {
    channel = ::util::tl::FindWithDefault(channel_mapping, channel, channel);
    is_mobile = meta::channel::IsMobile(meta::channel::GetChannel(channel));
  } else {
    channel = "web";
  }

  // Parse the the CGI params.
  // TODO(pramodg, ian): Check if URL CGI params need to be merged with the CGI params or are they
  // already merged in the frontend.
  if (cgi_params.empty() && url.size()) {
    string cgi_str;
    if (NetworkUtil::ExtractCGIStr(url, &cgi_str))
      NetworkUtil::ParseCGIArgumentsUnstrict(cgi_str, &cgi_params);
  }

  return true;
}

}  // namespace logging
