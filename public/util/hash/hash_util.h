#include <string>
#include <cstring>

#ifndef _PUBLIC_UTIL_HASH_HASH_UTIL_H_
#define _PUBLIC_UTIL_HASH_HASH_UTIL_H_

namespace hash {

  //
  // string hashing -- case sensitive and case-insensitive
  //

  struct str_hash {
    inline size_t operator()(const char *s) const {
      // copied from backward/hash_fun.h
      unsigned long __h = 0;
      for (; *s; ++s) __h = 5 * __h + *s;
      return size_t(__h);
    }
  };

  struct str_casefold_hash {
    inline size_t operator()(const char *s) const {
      size_t hash_val = 0;
      // similar hash implementation as str_hash
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
    inline size_t operator()(const std::string& s) const {
      return str_casefold_hash()(s.c_str());
    }
  };
  struct string_casefold_eq {
    inline bool operator()(const std::string& s1, const std::string& s2) const {
      return str_casefold_eq()(s1.c_str(), s2.c_str());
    }
  };


  //
  // flat structure hashing
  //

  template<class T>
  struct flat_hash {
    inline size_t operator()(T n) const {
      std::string s(reinterpret_cast<const char *> (&n), sizeof(n));
      return std::hash<std::string>()(s);
    }
  };
  // special handling for pairs
  template<class T1, class T2>
  struct flat_hash<std::pair<T1, T2> > {
    inline size_t operator()(const std::pair<T1, T2>& n) const {
      std::string s1(reinterpret_cast<const char *> (&(n.first)), sizeof(n.first));
      std::string s2(reinterpret_cast<const char *> (&(n.second)), sizeof(n.second));
      return std::hash<std::string>()(s1 + s2);
    }
  };
  // special handling for strings -- hash it using its normal hash function
  template<>
  struct flat_hash<std::string> {
    inline size_t operator()(const std::string& n) const {
      return std::hash<std::string>()(n);
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

#endif  // _PUBLIC_UTIL_HASH_HASH_UTIL_H_
