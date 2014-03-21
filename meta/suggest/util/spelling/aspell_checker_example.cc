// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

#include <iostream>
#include <chrono>
#include <vector>

#include "util/init/main.h"
#include "aspell_checker.h"

FLAG_string(dictionary_file,
    "/home/share/data/search/suggest/current/spelling.aspell",
    "Master dictionary file to use.");

int init_main() {
  ASpellChecker spell_checker(gFlag_dictionary_file);
  for (;;) {
    string term;
    cout << "\nEnter a single term: ";
    cin >> term;
    int count = 10;
    vector<string> first = spell_checker(term);
    cout << "Suggestions: ";
    for (auto s: first) cout << s << ", ";
    cout << endl;
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < count; ++i) {
      vector<string> v = spell_checker(term);
      assert(v == first);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    cout << "Generated in " << dur_ms << "ms " << 1000.0 * count / dur_ms << " qps" << endl;
  }
}
