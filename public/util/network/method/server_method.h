// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)
// File created as part of refactoring rpcserver.h

// Base class for server methods.


#ifndef _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_H_
#define _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_H_

#include <unordered_map>
#include <unordered_set>

#include "base/common.h"
#include "util/network/method/server_method_handler.h"

struct tConnectionInfo;
extern string gFlag_session_cookie_name;
extern string gFlag_guest_cookie_name;

namespace network {

// Base class for all RPC server handlers.
class ServerMethod {
 public:
  virtual ~ServerMethod() {}

  // special constructor to handle the case when one server method calls another
  // (do not call directly; use CascadeServerMethod() template instead)
  void Cascade(const ServerMethod* caller);
  void Cascade(const void* caller) {
    Cascade(static_cast<const ServerMethod*>(caller));
  }

  void SetConnection(const tConnectionInfo* connection) {
    connection_ = connection;
  }
  const tConnectionInfo* connection() const { return connection_; }

  void GetCallerIP(string* s_IPaddr, in_addr_t* l_IPaddr = NULL) const;


  string GetAWSUserIP() const {
    string user_ip = GetHttpHeader("X-Forwarded-For");
    // in some cases, if a user goes through multiple load balancers (e.g. ELB + haproxy),
    // the X-Forwarded-For may have the value: 80.5.6.5, 10.123.1.101. If we detect a comma,
    // we grab the first ip and assume that is the user's source ip.
    if (user_ip.find(",") == string::npos) return user_ip;
    vector<string> ips;
    strutil::Split(user_ip, ips, ",");
    return (!ips.empty() && !strutil::Trim(ips[0]).empty()) ?
      strutil::Trim(ips[0]) : "";
  }

  // Get user IP by looking at AWS "X-Forwarded-For" header first.
  // If it doesn't exist, use caller IP obtained from socket.
  string GetUserIP() const {
    string ip = GetAWSUserIP();
    if (ip.empty()) GetCallerIP(&ip, NULL);
    return ip;
  }

  // Returns the user country based on the geo ip.
  // NOTE: This will return gFlag_server_method_default_country (= "US") by
  // default when geoip service fails. This will happen, for example, when
  // hitting local servers (e.g. titan) as the ip is internal and not public.
  string GetUserCountry() const;

  /*
  bool ConnectionClosedByPeer() const {
    ASSERT(connection_ != NULL);
    return NetworkUtil::ConnectionClosedByPeer(connection_->ear);
  }
  */

  // cookie management
  // (currently, we allow multiple input cookies, but only allow at most
  //  one output cookie)

  bool InputCookieEmpty() const {
    return input_.empty();
  }

  // Get/Set input cookies map.
  const unordered_map<string, string>& input_cookies() const { return input_; }
  void set_input_cookies(const unordered_map<string, string>& input) { input_ = input; }

  // retrieve an input cookie by name
  const string& GetCookie(const string& name) const {
    static string kDefault = "";
    auto i = input_.find(name);
    if (i != input_.end()) return i->second;
    return kDefault;
  }

  // conveince function to grab the session cookie
  const string& GetSessionId() const;

  // get user id or return 0 if user not logged in
  string GetUserId() const;

  // add an output cookie
  void SetCookie(const string& name, const string& value, int ttl,
                 const string& path, const string& domain, bool secure,
                 bool http_only);

  void SetCookie(const string& name, const string& value, int ttl = -1) {
    SetCookie(name, value, ttl, "/", "", false, true);
  }

  // sets the session cookie
  void SetSessionCookie(const string& session_id);

  void SetReferrer(const string& referrer) { referrer_ = referrer; }

  const string& GetHttpHeader(const string& key) const {
    static string kDefault = "";
    auto itr = http_header_.find(key);
    if (itr != http_header_.end())  return itr->second;
    return kDefault;
  }

  // Function to run the validator for the server method.
  virtual ServerMethodStatus RunValidate(ostream& out);

  // Returns true if a server method does not contain sensitive data and
  // live traffic to it can be recorded.
  virtual bool CanRecord() { return true; }

  // Returns true if the method allows empty request. i.e. no 'q' CGI param.
  virtual bool AllowEmptyRequest() const { return false; }

  // Check wheter this server method has been called internally or is from
  // an actual user.
  bool internal_call() const { return internal_call_; }

  // ----------------------------------------------------------------
  // methods below are for RPCServer internal use only
  // ----------------------------------------------------------------

  const string& input_cookie() const { return input_cookie_; }
  const vector<string> output_cookies() const {
    vector<string> cookies;
    for (auto& cookie : output_cookies_) cookies.push_back(cookie);
    return cookies;
  }
  const unordered_map<string, string>& arg_map() const { return arg_map_; }
  const unordered_map<string, string>& http_header() const { return http_header_; }
  const string& referrer() const { return referrer_; }

  // add the output cookie string directly. cookie must already be formatted, so in
  // most cases the SetCookie method is recommended instead
  void add_output_cookie(const string& c) { output_cookies_.insert(c); }
  // clears the output cookies
  void clear_output_cookies() { output_cookies_.clear(); }

  // keep a copy of the entire CGI variable map except "q" (which is already
  // implicitly passed in through input structure)
  void set_arg_map(const unordered_map<string, string>& arg_map) {
    for (const auto& p : arg_map) {
      if (p.first != "q") arg_map_[p.first] = p.second;
    }
  }

  void set_http_header(const unordered_map<string, string>& http_header) {
    http_header_ = http_header;
  }

  // Set wheter this server method has been called internally or is from
  // an actual user.
  void set_internal_call(bool internal = true) { internal_call_ = internal; }

  // set input cookies from an HTTP Cookie header line and sets the session cookie
  // if not already set
  void ParseInputCookie(const string& cookie_spec);

 protected:
  // This method is overriden by the server methods.
  // Returns true if the the method is behaving as expected and false otherwise.
  virtual bool Validate(ostream& out) {
    has_validator_ = false;
    return false;
  }

  string referrer_, input_cookie_;
  unordered_set<string> output_cookies_;
  unordered_map<string, string> input_;  // key-value pairs in input cookies
  unordered_map<string, string> arg_map_;  // key-value pairs in CGI arguments
  unordered_map<string, string> http_header_;  // key-value pairs in HTTP header
  const tConnectionInfo* connection_ = nullptr;
  bool has_validator_ = true;
  // Determines if the call is made internally or is coming from a real user.
  bool internal_call_ = true;

 private:
  // compute a session cookie for the user
  string GenerateSessionCookie();
};

template<class tNestedMethod, class tInput, class tOutput>
string CascadeServerCall(const ServerMethod* caller,
                         const tInput& req, tOutput* result,
                         vector<string> *output_cookies = NULL) {
  tNestedMethod nested;
  nested.Cascade(caller);  // pass internal parameters to nested method
  string ret = nested(req, result);     // cascade call
  if (output_cookies) {
    *output_cookies = nested.output_cookies();
  }
  // note: use CascadeServerCall_WithOutputCookie() (defined below) if caller
  //       needs to pass output cookie upstream.  It's not automatically
  //       supported in this function because "caller" is defined as a
  //       const pointer which prohibits add_output_cookie() method
  return ret;
}

template<class tNestedMethod, class tInput, class tOutput>
string CascadeServerCall_WithOutputCookie(ServerMethod* caller,
                                          const tInput& req, tOutput* result) {
  vector<string> output_cookies;
  string ret = CascadeServerCall(caller, req, result, &output_cookies);
  for (auto& cookie : output_cookies) {
    caller->add_output_cookie(cookie);  // pass back output cookies
  }
  return ret;
}

// Cascade a server method call for the given method_name.
// Note: In *rare* cases caller can be null where you simply want to invoke a new
// method call. However, this should not be used with functions invoked from
// a server method as they require CGI params, cookies etc. to be passed forward
// to function correctly.
template<class tInput, class tOutput>
string CascadeServerCall(const string& method_name,
                         const ServerMethod *caller,
                         const tInput& req, tOutput *result) {
  ServerMethodHandlerBase *method = ServerMethodHandlerCollection::GetMethod(method_name);
  ASSERT(method != nullptr) << "no valid registered method found: " << method_name;
  string err_msg = method->ProcessCascade(caller, &req, result);
  return err_msg;
}

}  // namespace network

#endif  // _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_H_
