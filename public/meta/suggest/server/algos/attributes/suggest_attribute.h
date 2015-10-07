// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The attribute suggestion algorithm.
// This is used to suggest an attribute for a selected suggestion.

#ifndef _META_SUGGEST_SERVER_ALGOS_ATTRIBUTES_SUGGEST_ATTRIBUTE_H_
#define _META_SUGGEST_SERVER_ALGOS_ATTRIBUTES_SUGGEST_ATTRIBUTE_H_

#include <unordered_map>
#include <vector>

#include "base/defs.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/algos/suggest_algo.h"
#include "util/serial/serializer.h"

namespace suggest {
namespace algo {

class SuggestAttribute : public SuggestAlgo {
 public:
  virtual ~SuggestAttribute() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return params_.FromJSON(opts); }

  // Initialize the class.
  virtual bool Initialize();

  // The interface function for different suggest Algos. The response is filled
  // with relevant results from the algorithm. If a counter is specified the
  // algorithm calls Notify() once valid results have been filled into the
  // response.
  virtual int GetCompletions(const SuggestRequest& request,
                             shared_ptr<SuggestResponse> response,
                             shared_ptr<SuggestAlgoContext> context) const;

 protected:
  // Returns the attributes for a given parent completion.
  virtual int GetAttributes(const SuggestRequest& request,
                            const Completion& parent,
                            shared_ptr<SuggestResponse> response) const;

  // Initializes the default completions.
  bool InitDefaultAttributes();

  // Prepares the child attribute completion from the parent completion. This typically involves
  // setting up the score, suggestion id, etc.
  virtual void PrepareChildCompletionFromParent(const Completion& parent, Completion* child) const;

  // The parameters struct to configure the algo.
  struct SuggestAttributeParams {
    string id;
    // The algo name to use to fetch attribute for each suggestion.
    string attribute_index_algo_name;

    // The max number of suggestions for which the attributes are fetched.
    int max_attribute_candidates = 3;

    SERIALIZE(DEFAULT_CUSTOM / id*1 / attribute_index_algo_name*2 / max_attribute_candidates*3);
  };

  const SuggestAttributeParams& params() const { return params_; }

  // The parameters for the algorithm.
  SuggestAttributeParams params_;

  // The index for the attributes.
  // This index must have a special key defined by
  // 'suggest_algo_attribute_default_key'. That is used when no value can be
  // found for the main suggestion id.
  shared_proxy attribute_index_algo_;

  // The default completions used when no attributes can be found for a given
  // completion.
  shared_ptr<SuggestResponse> default_attributes_;
};

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_ATTRIBUTES_SUGGEST_ATTRIBUTE_H_
