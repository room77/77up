// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_DECAY_TIME_DECAY_H_
#define _UTIL_DECAY_TIME_DECAY_H_

#include <chrono>

#include "base/defs.h"
#include "base/logging.h"

#include "util/time/decay/exponential_decay.h"
#include "util/time/duration.h"

namespace decay {

// Implements a time decay function.
// Given a timestamp/duration, computes the delta from the current time and evaluates decay
// incurred.
template <typename _Duration = ::util::time::day, typename _DecayType = HalfLifeDecay>
class TimeDecay {
 public:
  typedef _Duration Duration;
  typedef _DecayType DecayType;

  TimeDecay(const DecayType& decay,
            const Duration& now = chrono::duration_cast<Duration>(
                chrono::high_resolution_clock::now().time_since_epoch()))
     : decay_(decay), now_(now) {}

  TimeDecay(const DecayType& decay, const typename Duration::rep& now)
      : TimeDecay(decay, Duration(now)) {}

  // Returns the decay if we know how for back we are from now.
  double DeltaDecay(double delta) const {
    return decay_.Decay(delta);
  }

  // Returns the decay incurred in delta time units.
  double Decay(const Duration& duration) const {
    Duration delta = now_ - duration;
    return DeltaDecay(delta.count());
  }

  double Decay(const typename Duration::rep& duration) const {
    return Decay(Duration(duration));
  }

  // When decay_factor_ == 2, HalfLife() == MeanLifeTime().
  double HalfLife() const {
    return decay_.HalfLife();
  }

  double mean_life_time() const {
    return decay_.mean_life_time();
  }

 private:
  DecayType decay_;
  Duration now_;
};

}  // namespace decay


#endif  // _UTIL_DECAY_TIME_DECAY_H_
