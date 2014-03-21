// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/common/merge/merge_completions.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/util/suggest_utils.h"

namespace suggest {
namespace merge {

using suggest::util::GetAlgoNameFromType;

// Merges two completions by picking the one with the lower score.
class CompletionsMinMerger : public CompletionsMerger {
 public:
  bool Merge(Completion* left, const Completion& right) const {
    if (left->score > right.score) {
      string debug_info = " | < Algo: " + GetAlgoNameFromType(left->algo_type) +
           ", " + std::to_string(left->score) + " (" + left->debug_info + ")";

      *left = right;
      left->debug_info.append(debug_info);
    }
    return true;
  }
};
auto reg_CompletionsMinMerger = CompletionsMerger::bind("<",
    [](){ return new CompletionsMinMerger(); });


// Merges two completions by picking the one with the greater score.
class CompletionsMaxMerger : public CompletionsMerger {
 public:
  bool Merge(Completion* left, const Completion& right) const {
    if (left->score < right.score) {
      string debug_info = " | > Algo: " + GetAlgoNameFromType(left->algo_type) +
          ", " + std::to_string(left->score) + " (" + left->debug_info + ")";
      *left = right;
      left->debug_info.append(debug_info);
    }
    return true;
  }
};
auto reg_CompletionsMaxMerger = CompletionsMerger::bind(">",
    [](){ return new CompletionsMaxMerger(); });

// Merges two completions by merging adding the two completions.
class CompletionsAddMerger : public CompletionsMerger {
 public:
  bool Merge(Completion* left, const Completion& right) const {
    left->score += right.score;
    left->debug_info.append(" | + Algo: " + GetAlgoNameFromType(right.algo_type) +
                            ", " + std::to_string(right.score) + "(" + right.debug_info + ")");

    // Merge other data.
    left->algo_type |= right.algo_type;

    return true;
  }
};
auto reg_CompletionsAddMerger = CompletionsMerger::bind("+",
    [](){ return new CompletionsAddMerger(); });


// Merges two completions by merging multiplying the two completions.
class CompletionsMultiplyMerger : public CompletionsMerger {
 public:
  bool Merge(Completion* left, const Completion& right) const {
    left->score *= right.score;
    left->debug_info.append(" | * Algo: " + GetAlgoNameFromType(right.algo_type) +
                            ", " + std::to_string(right.score) + "(" + right.debug_info + ")");

    // Merge other data.
    left->algo_type |= right.algo_type;

    return true;
  }
};
auto reg_CompletionsMultiplyMerger = CompletionsMerger::bind("*",
    [](){ return new CompletionsMultiplyMerger(); });

}  // namespace merge
}  // namespace suggest
