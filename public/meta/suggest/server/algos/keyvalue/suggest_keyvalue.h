// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Generic key value algorithms that load a file and serves the index.


#ifndef _META_SUGGEST_SERVER_ALGOS_KEYVALUE_SUGGEST_KEYVALUE_H_
#define _META_SUGGEST_SERVER_ALGOS_KEYVALUE_SUGGEST_KEYVALUE_H_

#include <unordered_map>
#include <vector>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue_base.h"

namespace suggest {
namespace algo {

// Key value algos that use SuggestKeyValueMap to store the index.
class SuggestKeyValue : public SuggestKeyValueBase {
  typedef SuggestKeyValueBase super;

 public:
  // The format in which the key value file is indexed and stored in binary
  // format.
  typedef unordered_map<string, vector<CompletionIndexItem>> SuggestKeyValueMap;

  virtual ~SuggestKeyValue() {}

  // Initialize the class.
  virtual bool Initialize();

 protected:
  // All subclasses must implement this function to fill the response with the
  // completions corresponding to the request.
  virtual int FindCompletions(const SuggestRequest& request,
                              shared_ptr<SuggestResponse> response,
                              shared_ptr<SuggestAlgoContext> context) const;

  // The index for the basic suggest prefixes.
  unique_ptr<SuggestKeyValueMap> kv_map_;
};

// Key value algos that use SuggestKeyValueExMap to store the index.
class SuggestKeyValueEx : public SuggestKeyValueBase {
  typedef SuggestKeyValueBase super;

 public:
  // The format in which the extended key value file is indexed and stored in
  // binary format. This is only used if we need to have different score per
  // match rather than the global default score for the associated completion.
  typedef unordered_map<string,
      vector<CompletionIndexItemEx>> SuggestKeyValueExMap;

  virtual ~SuggestKeyValueEx() {}

  // Initialize the class.
  virtual bool Initialize();

 protected:
  // All subclasses must implement this function to fill the response with the
  // completions corresponding to the request.
  virtual int FindCompletions(const SuggestRequest& request,
                              shared_ptr<SuggestResponse> response,
                              shared_ptr<SuggestAlgoContext> context) const;

  // The index for the extended suggest prefixes.
  unique_ptr<SuggestKeyValueExMap> kv_ex_map_;
};

// The algo used for fetching values keyed by SuggestionId.
// Currently we reuse the keyValue* algos for this as SuggestionId is a string.
// However if this changes, we will have to implement these explicitly.
typedef SuggestKeyValue SuggestSgstIdValue;
typedef SuggestKeyValueEx SuggestSgstIdValueEx;

// NOTE: We chose not to templatize these classes to keep the implementation in
// the .cc file and not to have it dumped in the header file as it is included
// at multiple places.

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_KEYVALUE_SUGGEST_KEYVALUE_H_
