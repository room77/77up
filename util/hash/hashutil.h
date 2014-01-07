#include <string>

#include <cstring>
#include "base/defs.h"

#ifndef _PUBLIC_UTIL_HASH_HASHUTIL_H_
#define _PUBLIC_UTIL_HASH_HASHUTIL_H_

namespace hashutil {

  //
  // string hashing -- case sensitive and case-insensitive
  //

  struct str_hash {
    inline size_t operator()(const char *s) const {
      return hash<const char *>()(s);
    }
  };

  struct str_casefold_hash {
    inline size_t operator()(const char *s) const {
      size_t hash_val = 0;
      // similar hash implementation as STL hash<const char *>
      while (*s != '\0') {
        hash_val = hash_val * 5 + toupper(*s);
        s++;
      }
      return hash_val;
    }
  };

  struct str_eq {
    inline bool operator()(const char* s1, const char* s2) const {
      return strcmp(s1, s2) == 0;
    }
  };

  struct str_casefold_eq {
    inline bool operator()(const char* s1, const char* s2) const {
      return strcasecmp(s1, s2) == 0;
    }
  };

  // also support "string" type
  struct string_casefold_hash {
    inline size_t operator()(const string& s) const {
      return str_casefold_hash()(s.c_str());
    }
  };
  struct string_casefold_eq {
    inline bool operator()(const string& s1, const string& s2) const {
      return str_casefold_eq()(s1.c_str(), s2.c_str());
    }
  };


  //
  // flat structure hashing
  //

  template<class T>
  struct flat_hash {
    inline size_t operator()(T n) const {
      string s(reinterpret_cast<const char *> (&n), sizeof(n));
      return hash<string>()(s);
    }
  };
  // special handling for pairs
  template<class T1, class T2>
  struct flat_hash<pair<T1, T2> > {
    inline size_t operator()(const pair<T1, T2>& n) const {
      string s1(reinterpret_cast<const char *> (&(n.first)), sizeof(n.first));
      string s2(reinterpret_cast<const char *> (&(n.second)), sizeof(n.second));
      return hash<string>()(s1 + s2);
    }
  };
  // special handling for strings -- hash it using its normal hash function
  template<>
  struct flat_hash<string> {
    inline size_t operator()(const string& n) const {
      return hash<string>()(n);
    }
  };

  // equality comparison function of flat structures
  template<class T>
  struct flat_eq {
    bool operator()(T n1, T n2) const {
      return (n1 == n2);
    }
  };
}

#endif  // _PUBLIC_UTIL_HASH_HASHUTIL_H_
