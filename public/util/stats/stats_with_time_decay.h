// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_STATS_STATS_WITH_TIME_DECAY_H_
#define _UTIL_STATS_STATS_WITH_TIME_DECAY_H_

#include <memory>

#include "base/defs.h"
#include "util/time/decay/time_decay.h"
#include "util/stats/stats.h"

namespace stats {

// Utility class for subclasses needing a time decay object.
class StatsWithHalfLifeTimeDecay : public StatsInterface {
 public:
  explicit StatsWithHalfLifeTimeDecay(int half_life = 90) : half_life_(half_life) {}

 protected:
  typedef ::decay::TimeDecay<> TimeDecayType;

  virtual void SetupTimeDecay(uint64_t now) {
    time_decay_.reset(new TimeDecayType(TimeDecayType::DecayType(half_life_),
                                        GetDuration(now)));
  }

  TimeDecayType* time_decay() const {
    return time_decay_.get();
  }

  // Returns the right duration from the timestamp (in microseconds).
  TimeDecayType::Duration GetDuration(uint64_t timestamp) const {
    return std::chrono::duration_cast<typename ::decay::TimeDecay<>::Duration>(
        std::chrono::microseconds(timestamp));
  }

  // Computes the decay given a timestamp (in microseconds).
  double GetDecayAmount(uint64_t timestamp) const {
    return time_decay() != nullptr ? time_decay()->Decay(GetDuration(timestamp)) : 1;
  }

  // The time decay associated with the stat.
  // This is set when TimeDecayData is called.
  unique_ptr<TimeDecayType> time_decay_;

  // The half life for time decay.
  int half_life_ = 90;
};

}  // namespace stats


#endif  // _UTIL_STATS_STATS_WITH_TIME_DECAY_H_
