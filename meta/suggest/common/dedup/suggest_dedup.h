// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_COMMON_DEDUP_SUGGEST_DEDUP_H_
#define _META_SUGGEST_COMMON_DEDUP_SUGGEST_DEDUP_H_

#include <memory>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/factory/factory.h"

namespace suggest {
namespace dedup {

// The interface class for different suggest dedup algorithms.
class SuggestDedup : public Factory<SuggestDedup> {
 public:
  virtual ~SuggestDedup() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  // The interface function for different suggest dedup Algos. The completions
  // in the response are deduped and updated by the deduper.
  // Returns the number of suggestions deduped.
  // NOTE: Dedupers depend on the rank order of the suggestions i.e. they
  //       require that suggestions are already in sorted order with the most
  //       important suggestion at the top.
  virtual int Dedup(const SuggestRequest& request,
                    shared_ptr<SuggestResponse> response) const = 0;
};

}  // namespace dedup
}  // namespace suggest

#endif  // _META_SUGGEST_COMMON_DEDUP_SUGGEST_DEDUP_H_
