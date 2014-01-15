
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <atomic>

#include "util/network/util.h"

#include "base/external.h"
#include "util/network/dnslookup.h"
#include "util/time/utime.h"

FLAG_string(tmp_email_message_dir, "/data/tmp",
            "temporary file directory to store email message");

FLAG_string(email_from_address, "alert@room77.com",
            "email address to send message from");

FLAG_string(email_from_real_name, "Room77",
            "real name to be shown to email recipient");

namespace NetworkUtil {

bool ExtractCGIStr(const string& url, string *cgi, string *base) {
  size_t found = url.find("?");
  bool success = false;
  if (found != string::npos) {
    success = true;
    *cgi = url.substr(found + 1);
    if (base) *base = url.substr(0, found);
  }
  return success;
}

void ExtractPathStr(const string &url, string *path, string *host) {
  size_t scheme = url.find("://");
  size_t start = scheme == string::npos ? 0 : scheme+3;
  size_t found = url.find('/', start);
  if (found != string::npos) {
    *path = url.substr(found);
    if (host) *host = url.substr(0, found);
  } else {
    *path = "";
    if (host) *host = url;
  }
}

bool ParseUrl(const string& url, string *hostname, string *path, string *cgi) {
  std::size_t host_pos = url.find("://");
  if (host_pos == string::npos) return false;
  host_pos += 3; // go to the start of the host
  std::size_t path_pos = url.find("/", host_pos);
  if (path_pos == string::npos) return false;
  // extra the hostname
  *hostname = url.substr(host_pos, path_pos - host_pos);
  // extra the cgi and path
  std::size_t cgi_pos = url.find("?", path_pos + 1);
  if (cgi_pos == string::npos) {
    *path = url.substr(path_pos);
    *cgi = "";
  } else {
    *path = url.substr(path_pos, cgi_pos - path_pos);
    *cgi = url.substr(cgi_pos + 1);
  }
  return true;
}

string MakeHttpCookie(const string& cookie_string, int ttl,
                      const string& path, const string& domain,
                      bool secure, bool http_only) {
  if (cookie_string.empty())
    return "";

  stringstream ss;
  ss << cookie_string;
  if (ttl > 0)
    ss << "; expires=" << CookieExpirationTime(ttl);
  if (!path.empty())
    ss << "; path=" << path;
  if (!domain.empty())
    ss << "; domain=" << domain;
  if (secure)
    ss << "; secure";
  if (http_only)
    ss << "; HttpOnly";
  return ss.str();
}

string CookieExpirationTime(int ttl) {
  Time t = Time::Now() + ttl;
  LocalTime gmt = t.GetLocalTime(Timezone::GMT());
  stringstream ss;
  ss << gmt.GetDayOfWeek_Abbr() << ", " << gmt.get_day() << "-"
     << gmt.GetMonth_Abbr() << "-" << gmt.get_year() << " "
     << setw(2) << setfill('0') << gmt.get_hour() << ":"
     << setw(2) << setfill('0') << gmt.get_minute() << ":"
     << setw(2) << setfill('0') << gmt.get_second() << " GMT";
  return ss.str();
}

bool IsValidEmail(const string& email) {
  const char *address = email.c_str();
  int count = 0;
  const char *c, *domain;
  static const char rfc822_specials[] = "()<>@,;:\\\"[]";

  /* first we validate the name portion (name@domain) */
  for (c = address;  *c;  c++) {
    if (*c == '\"' && (c == address || *(c - 1) == '.' || *(c - 1) ==
                       '\"')) {
      while (*++c) {
        if (*c == '\"') break;
        if (*c == '\\' && (*++c == ' ')) continue;
        if (*c <= ' ' || *c >= 127) return false;
      }
      if (!*c++) return false;
      if (*c == '@') break;
      if (*c != '.') return false;
      continue;
    }
    if (*c == '@') break;
    if (*c <= ' ' || *c >= 127) return false;
    if (strchr(rfc822_specials, *c)) return false;
  }
  if (c == address || *(c - 1) == '.') return false;

  /* next we validate the domain portion (name@domain) */
  if (!*(domain = ++c)) return false;
  do {
    if (*c == '.') {
      if (c == domain || *(c - 1) == '.') return false;
      count++;
    }
    if (*c <= ' ' || *c >= 127) return false;
    if (strchr(rfc822_specials, *c)) return false;
  } while (*++c);

  return (count >= 1);
}

string SendEmail(const string& to_addr,
                 const string& subject,
                 const string& message) {

  string hostname = DNSUtil::Instance().MyHostname();
  string ip = NetworkUtil::MyIP4Address();
  tEmailRequest req;
  req.to_addr = to_addr;
  req.subject = subject;
  stringstream msg;
  msg << "Host: " << hostname << ", ip: " << ip << '\n'
      << message;
  req.message = msg.str();
  string error_msg = SendEmail(req);
  if (!error_msg.empty()) {
    LOG(INFO) << "Warning: unable to send email: " << error_msg;
  }
  return error_msg;
}

string SendEmail(const tEmailRequest& req) {
  // check for input validity
  for (int i = 0; i < req.subject.size(); i++)
    if (req.subject[i] < 32)
      return "Subject contains invalid characters";

  if (!IsValidEmail(req.to_addr))
    return "invalid email address";
  if (!(req.reply_to.empty()))
    if (!IsValidEmail(req.reply_to))
      return "invalid reply-to email address";
  if (req.message.empty())
    return "message cannot be empty";

  stringstream ss;
  ss << "mail -s " << strutil::EscapeString_C(req.subject)
     << " -a 'From: " << strutil::EscapeString_C(gFlag_email_from_real_name)
     << " <" << gFlag_email_from_address << ">'";
  if (!(req.reply_to.empty())) {
    string rt = "Reply-To: ";
    rt += req.reply_to;
    ss << " -a " << strutil::EscapeString_C(rt);
  }
  ss << " " << strutil::EscapeString_C(req.to_addr) << " < %i";

  string output;
  int status;
  bool success = External::Call(ss.str(), req.message, &output, &status);

  if (!success || status != 0) {
    LOG(INFO) << "Warning: 'mail' returns status code " << status;
  }
  if (!output.empty())
    LOG(INFO) << "Warning: mail output is: " << output;

  return "";
}

const string& MyIP4Address(bool unsafe_reset) {
  // Local functor that does the update.
  struct MyIP4AddressHelper {
    string operator()() {
      string output;
      int status;

      External::Call("/sbin/ifconfig | grep -o 'inet addr:[^ ]*' | "
                     "grep -v '127\\.0\\.0\\.1' | sed 's/.*://g' | head -n 1 | "
                     "tr -d '\r\n' > %o",
                     "", &output, &status);
      return output;
    }
  };
  // C++0x guarantees that the initialization is done in a thread safe manner
  // wihtout memory leaks, race conditions etc. This was not safe before c++0x.
  static string my_ip = MyIP4AddressHelper()();
  // This portion is not thread safe (when unsafe_reset is true).
  if (unsafe_reset) my_ip = MyIP4AddressHelper()();
  return my_ip;
}

string GetHostIP(const string& hostname) {
  if (hostname.empty()) return "";

  struct hostent* h;
  h = gethostbyname(hostname.c_str());
  if (h == nullptr) return "";
  return inet_ntoa(*(struct in_addr *)h->h_addr);
}

bool IsValidIP(const string& ip) {
  if (ip.empty()) return false;

  bool res = true;
  struct addrinfo hint;
  hint.ai_socktype = 0;
  hint.ai_protocol = 0;
  hint.ai_family = AF_UNSPEC;
  hint.ai_flags = AI_NUMERICHOST | AI_ADDRCONFIG;
  struct addrinfo* resolved = nullptr;
  if (getaddrinfo(ip.c_str(), NULL, &hint, &resolved) != 0) {
    VLOG(3) << "Invalid ip: " << ip;
    res = false;
  }
  freeaddrinfo(resolved);
  return res;
}

// Returns checksum for for IP headers.
// The checksum field is the 16-bit one's complement of the one's
// complement sum of all 16-bit words in the header.
// For purposes of computing the checksum, the value of the checksum field is
// zero.
uint16_t ComputeIPChecksum(uint16_t *addr, int len) {
  // Using a 32 bit accumulator (sum), add sequential 16 bit words to it,
  // and at the end, fold back all the carry bits from the top 16 bits into
  // the lower 16 bits.
  register uint32_t sum = 0;
  register uint16_t *w = addr;
  register uint32_t nleft = len;
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }

  // Check for odd byte length.
  uint16_t answer = 0;
  if (nleft == 1) {
    *(uint8_t*)(&answer) = *(uint8_t*)w;
    sum += answer;
  }

  // Add back carry outs from top 16 bits to low 16 bits.
  sum = (sum >> 16) + (sum & 0xffff);  // Add hi 16 to low 16.
  sum += (sum >> 16);  // Add carry and truncate to 16 bits.
  return ~sum;  // Return one's complement.
}

// Returns true if ping succeeds and false otherwise.
bool Ping(const string& destination) {
  if (destination.empty()) {
    LOG(INFO) << "Invalid destination";
    return false;
  }

  const string cmd = "ping -c 1 -w 1 " + destination + " 2>&1";
  string out;
  int status = RunPipedCommand(cmd, &out);
  if (status) {
    LOG(INFO) << cmd << " failed with status " << status << "\n" << out;
  }

  return status == 0;
}

}  // namespace NetworkUtil
