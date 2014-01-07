#ifndef _PUBLIC_UTIL_NETWORK_DNSLOOKUP_H_
#define _PUBLIC_UTIL_NETWORK_DNSLOOKUP_H_

#include <mutex>

#include "base/common.h"

class DNSUtil {
 protected:
  DNSUtil();
 public:
  ~DNSUtil() {}

  static DNSUtil& Instance() {  // singleton instance
    static DNSUtil the_one;
    return the_one;
  }

  bool LookupHost(const string& hostname, int portnum, string *ret);

  inline string MyHostname() const { return my_hostname_; }

 private:
  // Key is host:port.
  unordered_map<string, pair<time_t, string> > cache_;
  string my_hostname_;

  mutex mutex_hashmap_;
};

#endif  // _PUBLIC_UTIL_NETWORK_DNSLOOKUP_H_
