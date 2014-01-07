#include <netdb.h>
#include <unistd.h>
#include "util/string/strutil.h"
#include "util/network/dnslookup.h"

DNSUtil::DNSUtil() {
  // keep track of my own host name
  char hostname[512];
  memset(hostname, 0, 512);
  gethostname(hostname, 512);

  struct addrinfo hint;
  hint.ai_socktype = 0;
  hint.ai_protocol = 0;
  hint.ai_family = AF_UNSPEC;
  hint.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_CANONNAME;
  struct addrinfo *resolved = nullptr;
  if (getaddrinfo(hostname, NULL, &hint, &resolved) != 0) {
    VLOG(2) << "Unknown host: " << hostname;
    my_hostname_.clear();
  }
  else {
    if (resolved->ai_canonname != NULL)
      my_hostname_ = resolved->ai_canonname;
  }
  freeaddrinfo(resolved);
  VLOG(2) << "My host name is " << my_hostname_;
}

bool DNSUtil::LookupHost(const string& hostname, int portnum,
                         string *ret) {
  bool success = false;
  time_t current_time = time(NULL);
  string port = strutil::ToString(portnum);
  string cache_key = hostname + ":" + port;
  string result;
  {
    lock_guard<mutex> l(mutex_hashmap_);
    auto itr = cache_.find(cache_key);
    if (itr != cache_.end() && itr->second.first + 1200 > current_time)
      result = itr->second.second;  // use 20-minute TTL
  }

  if (!(result.empty())) {
    VLOG(3) << "Using cached DNS lookup for " << hostname;
    success = true;
  }
  else {
    VLOG(2) << "Performing DNS lookup for " << hostname;

    struct addrinfo *resolved;
    int state = getaddrinfo(hostname.c_str(), port.c_str(), NULL, &resolved);
    if (state != 0) {
      VLOG(2) << "Unknown host: " << hostname
             << ", error: " << gai_strerror(state);
      success = false;
    }
    else {
      success = true;

      VLOG(2) << "Found " << hostname;
      if (resolved->ai_canonname != NULL)
        VLOG(2) << "canonical: " << resolved->ai_canonname;

      result = string(reinterpret_cast<char *>(resolved->ai_addr),
                      resolved->ai_addrlen);

      freeaddrinfo(resolved);
    }

    if (success) {
      // store lookup result in cache
      lock_guard<mutex> l(mutex_hashmap_);
      cache_[cache_key] = make_pair(current_time, result);
    }
  }

  if (success)
    *ret = result;

  return success;
}

