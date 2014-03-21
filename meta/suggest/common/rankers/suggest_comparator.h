// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_COMMON_RANKERS_SUGGEST_COMPARATOR_H_
#define _META_SUGGEST_COMMON_RANKERS_SUGGEST_COMPARATOR_H_

#include <functional>

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"

namespace suggest {
namespace rank {

// The basic comparator for completions based on score.
struct greater_score : std::binary_function <Completion, Completion, bool> {
  bool operator() (const Completion& left, const Completion& right) const {
    if (left.score != right.score) return left.score > right.score;

    // Break ties (arbitrarily) based on the src type. This ensures the order to be stable.
    return left.suggestion->src_type < right.suggestion->src_type;
  }
};

// The main comparator used to rank completions.
struct better_completion : greater_score {
  typedef greater_score super;
  bool operator() (const Completion& left, const Completion& right) const {
    /* For now we simply compute by score. All other static fixing is severely error prone.
     * It would be great if we can keep it this way.
     *
     * Old Comments:
    // The basic logic we follow here is as follows:
    // 1. Prefix suggestions are always ranked above everything except
    //    (child suggestions). i.e. if one is a prefix and other is a child
    //    they are compared based on the score or other parameters.
    int prefix_algo_comp = bitwise_diff(left.algo_type, right.algo_type,
                                        kCompletionAlgoTypePrefix);

    if (prefix_algo_comp != 0) {
      int child_comp = child_diff(left, right);
      if (!(prefix_algo_comp * child_comp < 0))
        return prefix_algo_comp > 0 ? true : false;
    }*/

    return super::operator()(left, right);
  }

 private:
  // Returns the bitwise diff for the algo type.
  // The values are:
  //   +ve: If left has 'bit' set and right does not.
  //   -ve: If right has 'bit' set and left does not.
  //   0: Both left and right have same value for 'bit' (either set or unset).
  int bitwise_diff(SugggestionAlgoType left, SugggestionAlgoType right,
                   SugggestionAlgoType bit) const {
    return (left & bit) - (right & bit);
  }

  // Returns the diff based on whether one is child or not.
  // The values are:
  //   +ve: If left is a child but right is not.
  //   -ve: If right is child but left is not.
  //   0: If right and left are either both chidren or none of them are.
  int child_diff(const Completion& left, const Completion& right) const {
    return (left.parent_id.empty() ? 0 : 1) - (right.parent_id.empty() ? 0 : 1);
  }
};

}  // namespace rank
}  // namespace suggest


#endif  // _META_SUGGEST_COMMON_RANKERS_SUGGEST_COMPARATOR_H_
