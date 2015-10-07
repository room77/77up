// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_STATS_SIGNAL_STATS_H_
#define _UTIL_STATS_SIGNAL_STATS_H_

#include <map>
#include <mutex>

#include "base/defs.h"
#include "util/serial/serializer.h"
#include "util/stats/stats_metadata.h"
#include "util/stats/stats_with_time_decay.h"
#include "util/templates/container_util.h"

namespace stats {

// A signal stats class.
// This can handle stats for multiple signals at the same time.
// When we have multiple signals, we assume signal = '*' to be the aggregate of all the signals.
class SignalStats : public StatsWithHalfLifeTimeDecay {
 public:
  using StatsWithHalfLifeTimeDecay::StatsWithHalfLifeTimeDecay;
  using StatsWithHalfLifeTimeDecay::ToJSON;
  using StatsWithHalfLifeTimeDecay::FromJSON;

  // Map from key -> (map from sigal -> value).
  // Note: We want this to be a map so that we can do operations like lower_bound and stuff.
  typedef map<string, map<string, double>> KeySignalFreqMap;

  // This returns the unlocked key frequncy map. The client is responsible for making sure
  // no parallel writes are happening when this function is used. This is typically useful
  // at the end of processing where all the aggregated data simply needs to be dumped.
  const KeySignalFreqMap& unsafe_key_signal_freq_map() const { return key_freq_; }

  const BasicMetaData& meta_data(const string& signal = Star()) const {
    static const BasicMetaData kDefault;
    return ::util::tl::FindWithDefault(meta_data_, signal, kDefault);
  }

  BasicMetaData& meta_data(const string& signal = Star()) {
    return meta_data_[signal];
  }

  // Returns the number of keys in the stats.
  virtual int size() const { return unsafe_key_signal_freq_map().size(); }


  // The timestamp up to which the stats have already been collected.
  virtual uint64_t ReadUptoTime() const {
    return ReadUptoTimeForSignal(Star());
  }

  // The timestamp up to which the stats have already been collected for a given signal.
  virtual uint64_t ReadUptoTimeForSignal(const string& signal) const;

  // The value against which all things should be time decayed.
  // Returns false if the timestamp now is older than the timestamp associated with the data.
  virtual bool TimeDecayData(uint64_t now) {
    return TimeDecaySignalData(Star(), now);
  }

  // The value against which all things should be time decayed.
  virtual bool TimeDecaySignalData(const string& signal, uint64_t now);

  // Increment the key.
  virtual void IncrementKey(const string& key, double value = 1, bool update_metadata = true) {
    IncrementKeyForSignal(Star(), key, value, update_metadata);
  }

  // Increments the key where the value was reported at ta given timestamp (in microseconds).
  virtual void IncrementKeyWithTimestamp(const string& key, uint64_t timestamp,
                                         double value = 1, bool update_metadata = true) {
    IncrementKeyForSignalWithTimestamp(Star(), key, timestamp, value, update_metadata);
  }

  // Increment the key for a given signal.
  virtual void IncrementKeyForSignal(const string& signal, const string& key, double value = 1,
                                     bool update_metadata = true);

  // Increments the key for the given signal where the value was reported at a given
  // timestamp (in microseconds).
  virtual void IncrementKeyForSignalWithTimestamp(const string& signal, const string& key,
      uint64_t timestamp,  double value = 1, bool update_metadata = true);

  // Get the value of the key.
  virtual double GetKey(const string& key) const {
    return GetKeyForSignal(Star(), key);
  }

  // Get the value of the key for the given signal.
  virtual double GetKeyForSignal(const string& signal, const string& key) const;

  // Removes keys that are below the given threshold.
  virtual int RemoveKeysBelowThreshold(double threshold) {
    return RemoveKeysForSignalBelowThreshold(Star(), threshold);
  }

  // Removes keys that are below the given threshold for the given signal.
  virtual int RemoveKeysForSignalBelowThreshold(const string& signal, double threshold);

  // Serialization interface.
  bool FromBinary(istream& in);
  bool FromJSON(istream& in);
  void ToBinary(ostream& out) const;
  void ToJSON(ostream& out,
      const serial::JSONSerializationParams& params = serial::JSONSerializationParams()) const;

 protected:
  typedef map<string, BasicMetaData> SignalMetadataMap;

  // The map for the key value.
  KeySignalFreqMap key_freq_;

  // The metadata associated with each signal.
  SignalMetadataMap meta_data_;

  mutable std::mutex m_;
};

}  // namespace stats


#endif  // _UTIL_STATS_SIGNAL_STATS_H_
