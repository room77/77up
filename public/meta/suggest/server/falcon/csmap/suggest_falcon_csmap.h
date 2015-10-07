// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_FALCON_CSMAP_SUGGEST_FALCON_CSMAP_H_
#define _META_SUGGEST_SERVER_FALCON_CSMAP_SUGGEST_FALCON_CSMAP_H_

#include <algorithm>
#include <unordered_map>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/falcon/suggest_falcon.h"
#include "util/templates/container_util.h"
#include "util/thread/counters.h"

namespace suggest {
namespace falcon {

// The basic map for storing and maintaining all suggestions.
class SuggestFalconCSMap : public SuggestFalcon {
 public:
  typedef unordered_map<SuggestionId, shared_ptr<CompleteSuggestion>>
      CompleteSuggestionMap;

  virtual ~SuggestFalconCSMap() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return params_.FromJSON(opts); }

  // Initialize the class.
  virtual bool Initialize();

  // Find a given suggestion id.
  virtual shared_ptr<CompleteSuggestion> Find(const SuggestionId& id) const {
    static const shared_ptr<CompleteSuggestion> kDefault;
    return ::util::tl::FindWithDefault(suggestion_map_, id, kDefault);
  }

  // Fills the response with the pointers to complete suggestions.
  virtual void AddCompleteSuggestions(shared_ptr<SuggestResponse> response,
      shared_ptr< ::util::threading::Counter> counter) const;

 protected:
  // The parameters struct to configure the algo.
  struct SuggestFalconCSMapParams {
    // The name fo the falcon.
    string id;

    // The file name to read the index from.
    string file;
    SERIALIZE(DEFAULT_CUSTOM / id*1 / file*2);
  };

  int size() const { return suggestion_map_.size(); }

  const CompleteSuggestionMap& suggestion_map() const {
    return suggestion_map_;
  }

 private:
  const SuggestFalconCSMapParams& params() const { return params_; }

  // The parameters for the algorithm.
  SuggestFalconCSMapParams params_;

  CompleteSuggestionMap suggestion_map_;
};

}  // namespace falcon
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_FALCON_CSMAP_SUGGEST_FALCON_CSMAP_H_
