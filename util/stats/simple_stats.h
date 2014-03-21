// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_STATS_SIMPLE_STATS_H_
#define _UTIL_STATS_SIMPLE_STATS_H_

#include <map>
#include <mutex>

#include "base/defs.h"
#include "util/serial/serializer.h"
#include "util/stats/stats_metadata.h"
#include "util/stats/stats_with_time_decay.h"

namespace stats {

// A simple stats class.
class SimpleStats : public StatsWithHalfLifeTimeDecay {
 public:
  using StatsWithHalfLifeTimeDecay::StatsWithHalfLifeTimeDecay;
  using StatsWithHalfLifeTimeDecay::ToJSON;
  using StatsWithHalfLifeTimeDecay::FromJSON;

  // Map from key -> value.
  // Note: We want this to be a map so that we can do operations like lower_bound and stuff.
  typedef map<string, double> KeyFreqMap;

  // This returns the unlocked key frequncy map. The client is responsible for making sure
  // no parallel writes are happening when this function is used. This is typically useful
  // at the end of processing where all the aggregated data simply needs to be dumped.
  const KeyFreqMap& unsafe_key_freq_map() const { return key_freq_; }

  const BasicMetaData& meta_data() const { return meta_data_; }

  // Returns the number of keys in the stats.
  virtual int size() const { return unsafe_key_freq_map().size(); }


  // The timestamp up to which the stats have already been collected.
  virtual uint64_t ReadUptoTime() const;

  // The timestamp(in microseconds) against which all things should be time decayed.
  // Returns false if the timestamp now is older than the timestamp associated with the data.
  virtual bool TimeDecayData(uint64_t now);

  // Increment the key.
  virtual void IncrementKey(const string& key, double value = 1, bool update_metadata = true);

  // Increments the key where the value was reported at ta given timestamp (in microseconds).
  virtual void IncrementKeyWithTimestamp(const string& key, uint64_t timestamp,
                                         double value = 1, bool update_metadata = true);

  // Get the value of the key.
  virtual double GetKey(const string& key) const;

  // Removes keys that are below the given threshold.
  virtual int RemoveKeysBelowThreshold(double threshold);

  // Serialization interface.
  bool FromBinary(istream& in);
  bool FromJSON(istream& in);
  void ToBinary(ostream& out) const;
  void ToJSON(ostream& out,
      const serial::JSONSerializationParams& params = serial::JSONSerializationParams()) const;

 protected:
  // The map for the key value.
  KeyFreqMap key_freq_;

  // The metadata associated with the stats.
  BasicMetaData meta_data_;

  mutable std::mutex m_;
};

}  // namespace stats

#endif  // _UTIL_STATS_SIMPLE_STATS_H_
