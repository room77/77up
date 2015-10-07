// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_DECAY_EXPONENTIAL_DECAY_H_
#define _UTIL_DECAY_EXPONENTIAL_DECAY_H_

#include <cmath>

#include "base/defs.h"

namespace decay {

// Implements a simple exponential decay function.
class ExponentialDecay {
 public:
  ExponentialDecay(double mean_life_time = 90, const double decay_factor = std::exp(1.0))
      : mean_life_time_(mean_life_time), decay_factor_(decay_factor) {}

  // Returns the decay incurred in delta time units.
  double Decay(double delta) const {
    return pow(decay_factor_, - delta / mean_life_time());
  }

  // When decay_factor_ == 2, HalfLife() == MeanLifeTime().
  double HalfLife() const {
    return log(2) * mean_life_time() / log(decay_factor_);
  }

  double mean_life_time() const {
    return mean_life_time_;
  }

 private:
  double mean_life_time_ = 0;  // lambda = 1 / mean life time
  double decay_factor_ = 0;  // the base exponent
};

// Utility specialization for half life decay.
class HalfLifeDecay : public ExponentialDecay {
 public:
  explicit HalfLifeDecay(double half_life_time = 90) : ExponentialDecay(half_life_time, 2) {}
};

}  // namespace decay


#endif  // _UTIL_DECAY_EXPONENTIAL_DECAY_H_
