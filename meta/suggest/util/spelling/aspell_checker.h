// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

#ifndef _META_SUGGEST_SPELLING_ASPELL_CHECKER_H_
#define _META_SUGGEST_SPELLING_ASPELL_CHECKER_H_

#include <aspell.h>
#include <base/common.h>
#include <cassert>
#include <iostream>
#include <vector>

// Spell checker using aspell library and our custom dictionary.
// Thread safe iff the instance is used by a single thread.
class ASpellChecker {
 public:
  ASpellChecker(const string& dictionary_file) {
    config_ = new_aspell_config();
    aspell_config_replace(config_, "lang", "en_US");
    aspell_config_replace(config_, "master", dictionary_file.c_str());
    aspell_config_replace(config_, "sug-mode", "ultra");
    AspellCanHaveError* possible_err = new_aspell_speller(config_);
    speller_ = 0;
    ASSERT(aspell_error_number(possible_err) == 0)
        << (aspell_error_message(possible_err));
    speller_ = to_aspell_speller(possible_err);
  }

  ~ASpellChecker() {
    delete_aspell_speller(speller_);
    delete_aspell_config(config_);
  }

  // Return correction suggestions. Empty if word is in dictionary.
  vector<string> operator()(const string& term) {
    vector<string> ret;
    if (!aspell_speller_check(speller_, term.c_str(), -1)) {
      const AspellWordList* suggestions = aspell_speller_suggest(speller_, term.c_str(), -1);
      AspellStringEnumeration* elements = aspell_word_list_elements(suggestions);
      const char* w;
      while ((w = aspell_string_enumeration_next(elements)) != nullptr)
        ret.push_back(w);
      delete_aspell_string_enumeration(elements);
    }
    return ret;
  }

 private:
  AspellConfig* config_;
  AspellSpeller* speller_;
};

#endif
