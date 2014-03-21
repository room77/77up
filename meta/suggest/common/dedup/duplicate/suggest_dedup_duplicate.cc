// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include <unordered_set>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/common/dedup/suggest_dedup.h"
#include "util/factory/factory.h"
#include "util/factory/factory_extra.h"

FLAG_string(suggest_dedup_duplicate_params, "",
            "Default parameters for suggest duplicate deduper.");

namespace suggest {
namespace dedup {

// Class to dedup duplicate suggestions.
class SuggestDedupDuplicate : public SuggestDedup {
 public:
  virtual ~SuggestDedupDuplicate() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  virtual int Dedup(const SuggestRequest& request,
                    shared_ptr<SuggestResponse> response) const {
    int deduped = response->completions.size();
    unordered_set<SuggestionId> seen;
    auto iter = remove_if(response->completions.begin(), response->completions.end(),
        [&seen](const Completion& completion) -> bool {
           if (seen.find(completion.suggestion_id) != seen.end()) return true;
           seen.insert(completion.suggestion_id);
           return false;
        });

    // Resize the completions if the
    if (iter != response->completions.end())
      response->completions.resize(iter - response->completions.begin());

    deduped -= response->completions.size();
    return deduped;
  }
};

// The domain boost twiddler.
auto reg_suggest_dedup_duplicate = SuggestDedup::bind(
    "suggest_dedup_duplicate",
    InitializeConfigureConstructor<SuggestDedupDuplicate, string>(
        gFlag_suggest_dedup_duplicate_params));


}  // namespace dedup
}  // namespace suggest

