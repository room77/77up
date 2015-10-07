// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin, Kyle Konrad

#ifndef _META_RANK_OPERATOR_H_
#define _META_RANK_OPERATOR_H_

#include <functional>
#include "base/common.h"
#include "util/factory/factory.h"

namespace meta {
namespace rank {

class Operator : public Factory<Operator> {
 public:
  Operator(function<float(float, float)> func) : func_(func) {}

  // Returns a binary function suitable for combining existing scores with the
  // scores produced by this ranker. Typical operators are plus and multiplies.
  // In all cases, scores produced by this ranker is multiplied with the weight
  // of the ranker (which the caller knows). Caller, than inquire the operator
  // function and apply merge the weighted scores with existing scores using the
  // operator. Typically all additive rankers must go first in the pipeline
  // followed by all multiplicative rankers.
  virtual function<float(float, float)> operator()() const { return func_; }

 private:
  function<float(float, float)> func_;
};

}  // namespace rank
}  // namespace meta

#endif
