// Copyright 2012 Room77, Inc.
// Author: Nicholas Edelman

// simple start stop timer
// expected usage:
// SimpleTimer t;
// t.Start();
// ... do something awesome ...
// LOG(INFO) << "awesome op time ms: " << t.GetIntermediateDuration();
// ... do something more awesome ...
// t.End();
// LOG(INFO) << "awesome op time ms: " << t.GetDurationMilliSec();
// LOG(INFO) << "awesome op time sec: " << t.GetDurationSec();
// t.Start(); // start recording another awesome op

#ifndef _PUBLIC_UTIL_TIME_SIMPLE_TIMER_H_
#define _PUBLIC_UTIL_TIME_SIMPLE_TIMER_H_

#include <chrono>

#include "base/common.h"
#include "util/time/duration.h"

namespace util {
namespace time {

class SimpleTimer {
 public:
  // start the timer. invalidates the stop
  void Start() {
    started_ = true;
    stopped_ = false;
    start_ = std::chrono::high_resolution_clock::now();
  }
  // stop the timer. can only be done once per Start
  void Stop() {
    if (!stopped_) {
      stopped_ = true;
      end_ = std::chrono::high_resolution_clock::now();
    }
  }
  // alias for Stop
  void End() { Stop(); }

  // Returns duration between start and stop.
  // If the timer was not started and stopped before this function is called,
  // -1 is returned.
  template <typename T = millisecond>
  typename T::rep GetDuration() {
    if (!started_ || !stopped_) return -1;
    auto elapsed = std::chrono::duration_cast<T>(end_ - start_);
    return elapsed.count();
  }

  // Returns the intermediate duration between start and current time.
  // If the timer was not started before this function is called,
  // -1 is returned.
  template <typename T = millisecond>
  typename T::rep GetIntermediateDuration() {
    if (!started_) return -1;
    auto elapsed = std::chrono::duration_cast<T>(
        std::chrono::high_resolution_clock::now() - start_);
    return elapsed.count();
  }

  double GetDurationMicroSec() { return GetDuration<microsecond>(); }
  double GetDurationMilliSec() { return GetDuration<millisecond>(); }
  double GetDurationSec() { return GetDuration<second>(); }
  double GetDurationMinute() { return GetDuration<minute>(); }
  double GetDurationHour() { return GetDuration<hour>(); }

 private:
  bool started_ = false, stopped_ = true;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_, end_;
};

// Utility timer to log the time between begin and end of a scope.
template <typename T = millisecond>
struct ScopedTimer {
  ScopedTimer(const string& str, int log_level = 1) : str_(str),
      log_level_(log_level) {
    timer_.Start();
  }

  ~ScopedTimer() {
    timer_.Stop();
    VLOG(log_level_) << " Time: " << str_ << ": " << timer_.GetDuration<T>();
  }

  const string str_;
  const int log_level_;
  SimpleTimer timer_;
};

typedef ScopedTimer<microsecond> ScopedMicroSecondsTimer;
typedef ScopedTimer<millisecond> ScopedMilliSecondsTimer;
typedef ScopedTimer<second> ScopedSecondsTimer;
typedef ScopedTimer<minute> ScopedMinutesTimer;
typedef ScopedTimer<hour> ScopedHoursTimer;

} // namespace time
} // namespace util


#endif  // _PUBLIC_UTIL_TIME_SIMPLE_TIMER_H_
