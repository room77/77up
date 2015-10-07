// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_COMMON_MERGE_MERGE_COMPLETIONS_H_
#define _META_SUGGEST_COMMON_MERGE_MERGE_COMPLETIONS_H_

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/factory/factory.h"

namespace suggest {
namespace merge {

// Interface class to merge completions.
class CompletionsMerger : public Factory<CompletionsMerger> {
 public:
  virtual ~CompletionsMerger() {}

  // Merges the left and the right completion and updates the left completion
  // with the new data.
  // Returns true on success.
  virtual bool Merge(Completion* left, const Completion& right) const = 0;
};

}  // namespace merge
}  // namespace suggest


#endif  // _META_SUGGEST_COMMON_MERGE_MERGE_COMPLETIONS_H_
