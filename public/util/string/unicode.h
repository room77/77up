// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

// Read http://userguide.icu-project.org/transforms/general

#ifndef _PUBLIC_UTIL_STRING_UNICODE_H_
#define _PUBLIC_UTIL_STRING_UNICODE_H_

#include <string>
#include <unicode/translit.h>
#include <memory>
#include <cassert>
#include <cstdlib>    // for mbsrtowcs

namespace unicode {

class Transliterator {
 public:
  Transliterator(const char* spec) {
    UParseError perr = { 0 };
    UErrorCode status = U_ZERO_ERROR;
    trans_.reset(icu::Transliterator::createInstance(spec, UTRANS_FORWARD, perr, status));
    assert(trans_ != NULL && !U_FAILURE(status));
  }

  std::string operator()(const std::string& input) {
    UnicodeString ustr(input.c_str());
    trans_->transliterate(ustr);
    // Allocate a big enough initial buffer for worst case scenarios.
    std::string output(ustr.length() * 5, ' ');
    output.resize(ustr.extract(0, ustr.length(), &output[0], output.size()));
    return output;
  }
 protected:
  std::unique_ptr<icu::Transliterator> trans_;
};

struct Lower : public Transliterator {
  Lower() : Transliterator("Lower;") {}
};
struct Upper : public Transliterator {
  Upper() : Transliterator("Upper;") {}
};
struct RemoveAccent : public Transliterator {
  RemoveAccent() : Transliterator("NFKD; [:M:] Remove; NFKC;") {}
};
struct LowerRemoveAccent : public Transliterator {
  LowerRemoveAccent() : Transliterator("Lower; NFKD; [:M:] Remove; NFKC;") {}
};
struct UpperRemoveAccent : public Transliterator {
  UpperRemoveAccent() : Transliterator("Upper; NFKD; [:M:] Remove; NFKC;") {}
};
struct NormalizeForIndexing : public Transliterator {
  // We need multiple Lower / Upper to handle cases where case change may
  // depend on language (e.g. Turkish dotless i). May not be correct, but we
  // will at least get to a consistent "canonical" form with mixed case input.
  NormalizeForIndexing() : Transliterator("Lower; NFKD; [:Nonspacing Mark:] Remove; NFKC;") {}
};

// Canonicalize the string for fuzzy comparison. We just lowercase the string,
// remove unicode accents etc. as much as possible, and convert a handful of
// characters to spaces. In the future we may consider stemming or more.
struct Canonicalize {
  std::string operator()(const std::string& s) {
    static const std::string remove_list("-+/&?,.");
    static unicode::LowerRemoveAccent lower_case;
    std::string str;
    str.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
      const char c = s[i];
      if (remove_list.find(c) != std::string::npos) {
        str += ' ';
      } else if (c == '\'') {
        // Skip
      } else {
        str += c;
      }
    }
    return lower_case(str);
  }
};

// Convert is temporarily needed until libstdc++ catches up with C++11.
// We only support utf8 string to wstring conversion. Assert fail for the rest.

// WARNING! std::setlocale(LC_ALL, "en_US.utf8"); must be called in order for
// Converter to work properly. Depending on unicode library will achieve this
// including this header only would compile but all conversions would fail.
template<class Input, class Output>
struct Convert {
  Output operator()(const Input& in) { assert(false); return Output(); }
};

template<>
struct Convert<std::string, std::wstring> {
  std::wstring operator()(const std::string& in) {
    static_assert(sizeof(wchar_t) == 4, "wchar_t is not 4 bytes long");
    std::wstring out(in.size() * 6 + 1, L'\0');
    std::mbstate_t state = std::mbstate_t();
    const char* src = &in[0];
    size_t size = mbsrtowcs(&out[0], &src, out.size(), &state);
    if (size != size_t(-1)) out.resize(size); else out.clear();
    return out;
  }
};

}  // Unicode

#endif  // _PUBLIC_UTIL_STRING_UNICODE_H_
