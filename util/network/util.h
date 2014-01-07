#ifndef _PUBLIC_UTIL_NETWORK_UTIL_H_
#define _PUBLIC_UTIL_NETWORK_UTIL_H_

#include <netdb.h>

#include "base/common.h"
#include "util/serial/serializer.h"

extern string gFlag_tmp_email_message_dir;

namespace NetworkUtil {

  /*
// check if the connection is closed by the other side
bool ConnectionClosedByPeer(int socket);
*/

// extract CGI string
// @return true if the cgi string was extracted successfully
bool ExtractCGIStr(const string& url, string *cgi, string *base=NULL);

// extract path string
void ExtractPathStr(const string& url, string *path, string *host=NULL);

// Parse CGI arguments and put results into an associative container.
template<class M>
bool ParseCGIArguments(const string& raw_args, M *arg_map) {
  arg_map->clear();
  const char *s = raw_args.c_str();
  const char *end = s + raw_args.size();

  // IE does not strip content after '#' sign while Firefox does.
  // We remove everything after # sign here.
  const char *pound_sign = strchr(s, '#');
  string truncated;
  if (pound_sign != NULL) {
    truncated = raw_args.substr(0, pound_sign - s);
    s = truncated.c_str();
    end = s + truncated.size();
  }

  s = strutil::SkipSpaces(s);
  while (s < end) {
    // look for '='
    const char *equal = strchr(s, '=');
    if (equal == NULL)
      break;
    if (equal == s)
      return false;  // empty variable name is not allowed

    const char *amp = strchr(s, '&');
    if (amp == NULL)
      amp = end;  // this is the last variable
    if (amp <= equal)
      return false;  // & cannot appear before =

    string name = string(s, equal - s);
    if (arg_map->find(name) != arg_map->end())
      return false;  // cannot have duplicate variable names

    string raw_value(equal + 1, amp - (equal + 1));

    // put the value into argument map
    (*arg_map)[name] = strutil::UnescapeString_CGI(raw_value);

    s = amp;
    if ((*s) == '&')
      s++;
  }
  return (s == end);  // cannot have extra characters after the end
}


// Generate CGI string from an associative container of (key,value) pairs
template<class M>
string GenerateCGIString(const M& arg_map) {
  string ret;
  for (auto it = arg_map.begin(); it != arg_map.end(); ++it) {
    if (!ret.empty()) ret.push_back('&');
    ret.append(strutil::EscapeString_CGI(it->first));
    ret.push_back('=');
    ret.append(strutil::EscapeString_CGI(it->second));
  }
  return ret;
}

// Backwards compatibility version.
template<class M>
void GenerateCGIString(const M& arg_map, string *output) {
  *output = GenerateCGIString(arg_map);
}

// parse CGI arguments and put results into a map but don't be picky about it
// always returns true
template <class M>
bool ParseCGIArgumentsUnstrict(const string& raw_args, M *arg_map) {
  arg_map->clear();
  const char *s = raw_args.c_str();
  const char *end = s + raw_args.size();

  // IE does not strip content after '#' sign while Firefox does.
  // We remove everything after # sign here.
  const char *pound_sign = strchr(s, '#');
  string truncated;
  if (pound_sign != nullptr) end = pound_sign;

  s = strutil::SkipSpaces(s);
  while (s < end) {
    // look for '='
    const char *equal = strchr(s, '=');
    if (equal == nullptr || equal >= end)
      break;
    if (equal == s) { // empty variable name is not allowed
      ++s;
      continue;
    }

    const char *amp = strchr(s, '&');
    if (amp == nullptr || amp > end) amp = end;  // this is the last variable
    if (amp > equal) { // only valid if & is after =
      string name = string(s, equal - s);
      string raw_value(equal + 1, amp - (equal + 1));

      // put the value into argument map
      (*arg_map)[name] = strutil::UnescapeString_CGI(raw_value);
    }
    s = amp + 1;
  }
  return true;
}

// @param url - a full url with http://$host$path?$cgi
// @return true if the url could be parsed successfully. false otherwise
// e.g. for http://foo.com/bar?a=b, then hostname=foo.com, path=/bar and cgi="a=b"
bool ParseUrl(const string& url, string *hostname, string *path, string *cgi);

string MakeHttpCookie(const string& cookie_string, int ttl,
                      const string& path, const string& domain,
                      bool secure, bool http_only);
inline string MakeHttpCookie(const string& cookie_string, int ttl) {
  return MakeHttpCookie(cookie_string, ttl, "/", "", false, true);
}
string CookieExpirationTime(int ttl);

struct tEmailRequest {
  // string session_id;
  string reply_to;
  string to_addr;
  string subject;
  string message;
  SERIALIZE(to_addr*1 / reply_to*2 / subject*3 / message*4);
};

/*
  check if a string is a valid email address
  attempts to follow specifications in RFC 822 - not fully compliant
  code from http://www.oreillynet.com/network/excerpt/spcookbook_chap03/index3.html
*/
bool IsValidEmail(const string& addr);

// convenience function for send email and prepends the message with the
// hostname and ip
string SendEmail(const string& to_addr,
                 const string& subject,
                 const string& message);
// a quick way to send an email using a system call to `mail`
// see network/mail/mailer.h and subclasses for a more scalable approach
string SendEmail(const tEmailRequest& req);

// Returns the IP v4 address of the machine. Assumes ifconfig, grep, sed, tr
// and head are available. The value is cached first time the function is
// called in a thread safe manner. If there are multiple network interfaces,
// the first non-loopback interface is returned. Empty string means failure /
// machine did not have an IP at the time of first call.
// If the IP number is assigned or changed after the first call for some
// reason, you may call it with unsafe_reset bit set to recheck the IP. Unsafe
// reset usage is not thread safe, however and needs to be synchronized.
const string& MyIP4Address(bool unsafe_reset = false);

// Returns the ip for the given hostname.
string GetHostIP(const string& hostname);

// Returns true if the hostname is valid.
inline bool IsValidHost(const string& hostname) {
  return GetHostIP(hostname).size();
}

// Returns true if it is a valid IP (v4 or v6) address.
bool IsValidIP(const string& ip);

// Computes the IP header checksum.
uint16_t ComputeIPChecksum(uint16_t *addr, int len);

// Returns true if ping succeeds for the given destination and false otherwise.
bool Ping(const string& destination);

//---------------------------------------------------------------//
//      Convenience functions for HTTP request with retry
//---------------------------------------------------------------//

// @param client_prototype
//    - if necessary, this can be adjusted with parameters like
//      user_agent and timeout
// @param Parse (function) - should return true iff raw_reply seems good

template<class Client>
bool GetWithRetry(
    const Client& client_prototype,
    const string& host, int port, const string& path, int num_retries,
    std::function<bool (const string&)> Parse =
        [] (const string&) {return true;}) {

  int tries = -1;
  for (; tries < num_retries; ++tries) {
    string raw_reply;
    int status_code;
    Client h(client_prototype);

    if (!h.HttpGet(host, port, path, &status_code, &raw_reply)
        || status_code != 200) {
      VLOG(3) << "Error in http get. Status Code: " << status_code;
      VLOG(3) << "Reply: " << raw_reply;
      continue;
    }
    VLOG(3) << "Reply size: " << raw_reply.size();
    VLOG(3) << raw_reply;
    if (Parse(raw_reply)) return true;
    continue;
  }
  return false;
}

template<class Client>
bool PostWithRetry(
    const Client& client_prototype,
    const string& host, int port, const string& path, const string& message,
    int num_retries,
    std::function<bool (const string&)> Parse =
        [] (const string&) {return true;}) {

  int tries = -1;
  for (; tries < num_retries; ++tries) {
    string raw_reply;
    int status_code;
    Client h(client_prototype);
    if (!h.HttpPost(host, port, path, message, &status_code, &raw_reply)
        || status_code != 200) {
      VLOG(3) << "Error in http get. Status Code: " << status_code;
      VLOG(3) << "Reply: " << raw_reply;
      continue;
    }
    VLOG(3) << "Reply size: " << raw_reply.size();
    VLOG(3) << raw_reply;
    if (Parse(raw_reply)) return true;
    continue;
  }
  return false;
}

}  // namespace NetworkUtil

#endif  // _PUBLIC_UTIL_NETWORK_UTIL_H_
