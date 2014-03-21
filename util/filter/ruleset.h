// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

// Generic framework to define chained rule sets (similar to typical firewalls).
// It can be used in various domains. See the unit test for example usage.

#ifndef _PUBLIC_UTIL_FILTER_RULESET_H_
#define _PUBLIC_UTIL_FILTER_RULESET_H_

#include <iostream>
#include <functional>
#include <atomic>
#include <future>

#include "util/string/strutil.h"
#include "util/factory/factory.h"
#include "util/serial/serializer.h"

namespace ruleset {

// Base class for individual or chained rules. We use strings for factory IDs
// and on-the-fly configuration of parameters.
template<class Input, class Output>
class Rule : public Factory<Rule<Input, Output>, string, string> {
 public:
  struct Spec { string id, params; SERIALIZE(id*1 / params*2); };
  typedef Input input_type;
  typedef Output result_type;
  virtual result_type operator()(const Input& in) const = 0;
};

// A rule set class that returns the value of the first matching rule (similar
// to most firewall rule sets or rule chains). A special value, unknown_value is
// used to denote that an individual rule did not "match" this case, and the
// next rule in the chain should be tried. Another special value, default_value
// will be returned if none of the rules "match".
template<class RuleType>
class FirstMatch : public Rule<typename RuleType::input_type, typename RuleType::result_type> {
 public:
  typedef typename RuleType::input_type input_type;
  typedef typename RuleType::result_type result_type;
  typedef vector<typename RuleType::Spec> spec_type;
  typedef std::function<spec_type(const string&)> spec_parser;

  static spec_parser& JsonParser() {
    static spec_parser parser =
      [](const string& s) {
        spec_type ret;
        ASSERT(serial::Serializer::FromJSON(s, &ret))
            << "Error parsing specs string: " << s;
        return ret;
      };
    return parser;
  }

  static spec_parser& CsvParser() {
    static spec_parser parser =
      [](const string& s) {
        spec_type ret;
        ASSERT(serial::Serializer::FromJSON(s, &ret))
            << "Error parsing specs string: " << s;
        return ret;
      };
    return parser;
  }

  FirstMatch(const string& rule_specs, const result_type& u, const result_type& d,
       spec_parser parser = JsonParser()) : unknown_value_(u), default_value_(d) {
    spec_type specs = parser(rule_specs);
    ASSERT(!specs.empty());
    for (auto& spec : specs) {
      auto rule = RuleType::make_shared(spec.id, spec.params);
      ASSERT(rule.get()) << "Error creating rule with id: " << spec.id
          << ", params: " << spec.params;
      rules_.push_back(rule);
    }
  }

  FirstMatch(const string& rule_specs, const result_type& u) : FirstMatch(rule_specs, u, u) {}

  result_type operator()(const input_type& input) const {
    result_type ret = unknown_value_;
    for (auto& rule : rules_) {
      ret = rule->operator()(input);
      if (ret != unknown_value_) return ret;
    }
    if (ret == unknown_value_) ret = default_value_;
    return ret;
  }

 protected:
  result_type unknown_value_;
  result_type default_value_;
  vector<typename RuleType::shared_proxy> rules_;
};

}

#endif  // _PUBLIC_UTIL_FILTER_RULESET_H_
