// Wrapper Author: B. Uygar Oztekin

// This library encapsulates Porter Stemming Algorithm into a more C++ friendly
// thread safe class. Original code is untouched except portions of it is put in
// an unnamed namespace and some function interface modified to use const char *
// instead of char * to avoid compiler warnings.

#ifndef _UTIL_PORTER_STEMMER_H_
#define _UTIL_PORTER_STEMMER_H_

#include <string>

struct stemmer;

class PorterStemmer {
 public:
  PorterStemmer();
  ~PorterStemmer();

  // Returns the stemmed version of the term.
  // Input string must containt exactly one term.
  std::string StemTerm(const std::string& term) const;

 private:
  stemmer* stemmer_;
};

#endif
