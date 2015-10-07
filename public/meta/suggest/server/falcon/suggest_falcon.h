// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The falcon library serves the suggest data.
// TODO(pramodg): Add sharding support in the future.


#ifndef _META_SUGGEST_SERVER_FALCON_SUGGEST_FALCON_H_
#define _META_SUGGEST_SERVER_FALCON_SUGGEST_FALCON_H_

#include <algorithm>
#include <unordered_map>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/factory/factory.h"
#include "util/factory/factory_extra.h"
#include "util/templates/container_util.h"
#include "util/thread/counters.h"

namespace suggest {
namespace falcon {

class SuggestFalcon : public Factory<SuggestFalcon, string, string> {
 public:
  virtual ~SuggestFalcon() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  // Find a given suggestion id.
  virtual shared_ptr<CompleteSuggestion> Find(const SuggestionId& id) const = 0;

  // Fills the response with the pointers to complete suggestions.
  virtual void AddCompleteSuggestions(shared_ptr<SuggestResponse> response,
      shared_ptr< ::util::threading::Counter> counter) const = 0;

  void AddCompleteSuggestions(shared_ptr<SuggestResponse> response) const {
    AddCompleteSuggestions(response, shared_ptr< ::util::threading::Counter>());
  }
};

}  // namespace falcon
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_FALCON_SUGGEST_FALCON_H_
