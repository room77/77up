// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)
// File created as part of refactoring rpcserver.h/cc

#include "util/network/method/server_method.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#include "base/common.h"
#include "base/strutil.h"
#include "geo/lib/geoip.h"
#include "util/network/netserver.h"
#include "util/network/util.h"
#include "util/md5/md5.h"
#include "util/random/random.h"

FLAG_string(session_cookie_name, "r77sessioncookie", "name of the session cookie");

FLAG_string(guest_cookie_name, "r77_guest", "name of the guest cookie");
// keep around for 10 years 10 * 365 * 24 * 60 * 60
FLAG_int(session_cookie_duration, 315360000, "number of seconds to keep the session cookie");

FLAG_string(server_method_default_country, "US",
            "The default country to use when geoip fails.");


namespace network {

void ServerMethod::Cascade(const ServerMethod* caller) {
  if (caller == nullptr) return;

  SetConnection(caller->connection());
  SetReferrer(caller->referrer());
  set_input_cookies(caller->input_cookies());
  set_arg_map(caller->arg_map());
  set_http_header(caller->http_header());
  set_internal_call(caller->internal_call());
};

void ServerMethod::GetCallerIP(string* s_IPaddr, in_addr_t* l_IPaddr) const {
  if (s_IPaddr && connection_) {
    // caller's IP address (string)
    *s_IPaddr = inet_ntoa(connection_->caller_id.sin_addr);
    if (l_IPaddr) {
      // caller's IP address (number)
      *l_IPaddr = inet_addr(s_IPaddr->c_str());
    }
  }
}

string ServerMethod::GetUserCountry() const {
  string country = geoip::IpToCountryCode(GetUserIP());
  if (country.empty()) country = gFlag_server_method_default_country;
  return country;
}

const string& ServerMethod::GetSessionId() const {
  return GetCookie(gFlag_session_cookie_name);
}

// get user id or return 0 if user not logged in
string ServerMethod::GetUserId() const {
  string cookie = GetCookie(gFlag_guest_cookie_name);
  if (cookie.empty()) return "";
  static const string kCookieDelimiter = "%";
  return cookie.substr(0, cookie.find(kCookieDelimiter));
}

// add an output cookie
void ServerMethod::SetCookie(const string& name, const string& value,
                             int ttl, const string& path,
                             const string& domain,
                             bool secure, bool http_only) {
  string s = (strutil::EscapeString_CGI(name) + "=" +
              strutil::EscapeString_CGI(value));
  add_output_cookie(NetworkUtil::MakeHttpCookie(s, ttl, path, domain,
                                                secure, http_only));
}

// set input cookies from an HTTP Cookie header line
void ServerMethod::ParseInputCookie(const string& cookie_spec) {
  input_cookie_ = (input_cookie_.empty()) ?
    cookie_spec : input_cookie_ + ";" + cookie_spec;
  const char* s = strutil::SkipSpaces(cookie_spec.c_str());
  while ((*s) > ' ') {
    const char* equal = strchr(s, '=');
    if (equal == NULL) break;
    const char* v = equal + 1;
    const char* end = strchr(v, ';');
    string name(s, equal - s);
    string value = (end == NULL ? string(v) : string(v, end - v));
    input_[name] = value;  // add input cookie (name, value)
    if (end == NULL) break;
    s = strutil::SkipSpaces(end + 1);
  }
  // check if the session cookie exists. if not, set it
  string session_cookie = GetSessionId();
  if (session_cookie.empty()) {
    session_cookie = GenerateSessionCookie();
    input_[gFlag_session_cookie_name] = session_cookie;
    // construct the cookie
    string session_cookie_spec = gFlag_session_cookie_name + "=" + session_cookie;
    input_cookie_ = (input_cookie_.empty()) ?
       session_cookie_spec : input_cookie_ + ";" + session_cookie_spec;

    SetCookie(gFlag_session_cookie_name, session_cookie, gFlag_session_cookie_duration,
              "/", "", false, false);
  }
}

// Function to run the validator for the server method.
ServerMethodStatus ServerMethod::RunValidate(ostream& out) {
  bool res = Validate(out);
  if (has_validator_) {
    return res ? kServerMethodStatusValid : kServerMethodStatusInvalid;
  }
  out << "Not implemented yet!" << endl;
  return kServerMethodStatusNotImplemented;
}

//
// === PRIVATE ===
//

string ServerMethod::GenerateSessionCookie() {
  MD5 md5(::util::random::GetUniqueString("session"));
  return md5.hexdigest();
}

}  // namespace network
