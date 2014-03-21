// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The attribute suggestion algorithm for locations.
// This is used to suggest a location attribute for a selected suggestion.

#ifndef _META_SUGGEST_SERVER_ALGOS_ATTRIBUTES_LOCATION_SUGGEST_ATTRIBUTE_LOCATION_H_
#define _META_SUGGEST_SERVER_ALGOS_ATTRIBUTES_LOCATION_SUGGEST_ATTRIBUTE_LOCATION_H_

#include "meta/suggest/server/algos/attributes/suggest_attribute.h"

#include "base/defs.h"
#include "meta/suggest/common/suggest_datatypes.h"

namespace suggest {
namespace algo {

// Class for returning the location attributes for a parent suggestion.
class SuggestLocationAttribute : public SuggestAttribute {
 public:
  virtual ~SuggestLocationAttribute() {}

 protected:
  // Prepares the child attribute completion from the parent completion. This typically involves
  // setting up the score, suggestion id, etc.
  virtual void PrepareChildCompletionFromParent(const Completion& parent, Completion* child) const;
};

}  // namespace algo
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_ALGOS_ATTRIBUTES_LOCATION_SUGGEST_ATTRIBUTE_LOCATION_H_
