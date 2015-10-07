// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

#include <iostream>
#include <string>
#include "ruleset.h"
#include "util/init/main.h"
#include "util/serial/serializer.h"

// A rule that matches if n is a multiple of num.
struct TestRule : public ruleset::Rule<int, int> {
  TestRule(const string& params = "") {
    if (!params.empty()) {
      ASSERT(serial::Serializer::FromJSON(params, this))
          << "Error parsing params: " << params;
    }
  }

  // Return values : 0: false, 1: true, 2: unkown.
  // Returns true if n is a multiple of num, unknown otherwise.
  int operator()(const int& n) const { return n % num == 0 ? match_value : 2; }

  int num = 1;
  int match_value = 1;
  SERIALIZE(DEFAULT_CUSTOM / num*1 / match_value*2);
};

auto bind_multiple_of = TestRule::bind("multiple_of", "{}",
    [](const string& params){ return new TestRule(params); } );

auto bind_rule_set = ruleset::FirstMatch<TestRule>::bind(
    "multiple_of_2_but_not_3",
    R"([
        { id:"multiple_of", params:"{ num:3, match_value:0 }" },
        { id:"multiple_of", params:"{ num:2, match_value:1 }" }
      ])",
    [](const string& params){
      return new ruleset::FirstMatch<TestRule>(params, 2, 0);
    }
);

int init_main() {
  auto ruleset = ruleset::Rule<int, int>::make_shared("multiple_of_2_but_not_3");
  ASSERT(ruleset.get());
  for (int i = 0; i < 15; ++i) {
    cout << "Rule: multiple_of_2_but_not_3, return value for " << i << ": "
         << ruleset->operator()(i) << endl;
  }
  return 0;
}
