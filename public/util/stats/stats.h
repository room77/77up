// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_STATS_STATS_H_
#define _UTIL_STATS_STATS_H_

#include <sstream>

#include "base/defs.h"
#include "util/factory/factory.h"
#include "util/serial/utils/serializer_util.h"

namespace stats {

enum {
  kDontUpdateMetaData = 0,
  kUpdateMetaData = 1,
};

class StatsInterface : public Factory<StatsInterface> {
 public:
  virtual ~StatsInterface() {}

  // Use to represent *.
  static const string& Star() {
    static const string kStar = "*";
    return kStar;
  }

  // Returns the number of keys in the stats.
  virtual int size() const = 0;

  // The timestamp up to which the stats have already been collected.
  virtual uint64_t ReadUptoTime() const = 0;

  // The timestamp up to which the stats have already been collected for a given signal.
  virtual uint64_t ReadUptoTimeForSignal(const string& signal) const {
    return ReadUptoTime();
  }

  // The timestamp(in microseconds) against which all things should be time decayed.
  // Returns false if the timestamp now is older than the timestamp associated with the data.
  virtual bool TimeDecayData(uint64_t now) = 0;

  // The timestamp(in microseconds) against which the signal should be time decayed.
  virtual bool TimeDecaySignalData(const string& signal, uint64_t now) {
    return TimeDecayData(now);
  }

  // Increment the key.
  virtual void IncrementKey(const string& key, double value = 1, bool update_metadata = true) = 0;

  // Increments the key where the value was reported at a given timestamp (in microseconds).
  virtual void IncrementKeyWithTimestamp(const string& key, uint64_t timestamp,
                                         double value = 1, bool update_metadata = true) {
    IncrementKey(key, value, update_metadata);
  }

  // Increment the key for a given signal.
  virtual void IncrementKeyForSignal(const string& signal, const string& key, double value = 1,
                                     bool update_metadata = true) {
    return IncrementKey(key, value, update_metadata);
  }

  // Increments the key for the given signal where the value was reported at a given
  // timestamp (in microseconds).
  virtual void IncrementKeyForSignalWithTimestamp(const string& signal, const string& key,
      uint64_t timestamp,  double value = 1, bool update_metadata = true) {
    IncrementKeyWithTimestamp(key, timestamp, value, update_metadata);
  }

  // Get the value of the key.
  virtual double GetKey(const string& key) const = 0;

  // Get the value of the key for the given signal.
  virtual double GetKeyForSignal(const string& signal, const string& key) const {
    return GetKey(key);
  }

  // Removes keys that are below the given threshold.
  virtual int RemoveKeysBelowThreshold(double threshold) = 0;

  // Removes keys that are below the given threshold for the given signal.
  virtual int RemoveKeysForSignalBelowThreshold(const string& signal, double threshold) {
    return RemoveKeysBelowThreshold(threshold);
  }

  // Serialization interface.
  virtual bool FromBinary(istream& in) = 0;
  virtual bool FromJSON(istream& in) = 0;

  virtual void ToBinary(ostream& out) const = 0;
  virtual void ToJSON(ostream& out,
      const serial::JSONSerializationParams& params = serial::JSONSerializationParams()) const = 0;

  string ToJSON(
      const serial::JSONSerializationParams& params = serial::JSONSerializationParams()) const {
    ostringstream out;
    ToJSON(out, params);
    return out.str();
  }

  bool FromJSON(const string& s) {
    istringstream in(s);
    return FromJSON(in);
  }
};

}  // namespace stats


#endif  // _UTIL_STATS_STATS_H_
