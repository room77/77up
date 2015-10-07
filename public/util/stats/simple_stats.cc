// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/stats/simple_stats.h"

#include <chrono>

#include "util/templates/container_util.h"

namespace stats {

uint64_t SimpleStats::ReadUptoTime() const {
  if (meta_data_.read_ahead_from_time == 0) return meta_data_.timestamp;
  return meta_data_.read_ahead_from_time;
}

bool SimpleStats::TimeDecayData(uint64_t now) {
  // Decay all values by the amount.
  lock_guard<std::mutex> l(m_);
  if (now < meta_data_.timestamp) return false;

  meta_data_.read_ahead_from_time = meta_data_.timestamp;
  meta_data_.timestamp = now;

  // Setup the decay object.
  SetupTimeDecay(now);

  meta_data_.clear();
  if (unsafe_key_freq_map().empty()) return true;

  // Decay all the previously recorded values appropriately.
  double decay_amount = GetDecayAmount(meta_data_.read_ahead_from_time);
  for (auto& p : key_freq_) {
    p.second *= decay_amount;
    meta_data_.Increment(p.second);
  }
  return true;
}

void SimpleStats::IncrementKey(const string& key, double value, bool update_metadata) {
  lock_guard<std::mutex> l(m_);
  key_freq_[key] += value;
  if (update_metadata) meta_data_.Increment(value);
}

void SimpleStats::IncrementKeyWithTimestamp(const string& key, uint64_t timestamp, double value,
                                            bool update_metadata) {
  // Check if the value is older than what we want to read from.
  if (timestamp < meta_data_.read_ahead_from_time) return;

  // Decay the value appropriately based on the timestamp.
  value *= GetDecayAmount(timestamp);

  // Increment the key with the value.
  IncrementKey(key, value, update_metadata);
}

// Get the value of the key.
double SimpleStats::GetKey(const string& key) const {
  lock_guard<std::mutex> l(m_);
  return ::util::tl::FindWithDefault(unsafe_key_freq_map(), key, 0);
}

// Removes keys that are below the given threshold.
int SimpleStats::RemoveKeysBelowThreshold(double threshold) {
  lock_guard<std::mutex> l(m_);

  meta_data_.clear();
  int count = size();
  for (auto iter = key_freq_.begin(); iter != key_freq_.end();) {
    if (iter->second < threshold) {
      iter = key_freq_.erase(iter);
      continue;
    }

    meta_data_.Increment(iter->second);
    ++iter;
  }
  return count - size();
}

// Serialization interface.
bool SimpleStats::FromBinary(istream& in) {
  lock_guard<std::mutex> l(m_);
  // Read the first metadata line.
  if (!serial::Serializer::FromBinary(in, &meta_data_)) return false;

  // Read the stats data.
  return serial::Serializer::FromBinary(in, &key_freq_);
}

bool SimpleStats::FromJSON(istream& in) {
  lock_guard<std::mutex> l(m_);
  // Read the first metadata line.
  if (!serial::Serializer::FromJSON(in, &meta_data_)) return false;

  // Read the stats data.
  return serial::Serializer::FromJSON(in, &key_freq_);
}

void SimpleStats::ToBinary(ostream& out) const {
  lock_guard<std::mutex> l(m_);
  // Read the first metadata line.
  serial::Serializer::ToBinary(out, meta_data_);
  // Read the stats data.
  serial::Serializer::ToBinary(out, key_freq_);
}

void SimpleStats::ToJSON(ostream& out, const serial::JSONSerializationParams& params) const {
  lock_guard<std::mutex> l(m_);
  // Read the first metadata line.
  serial::Serializer::ToJSON(out, meta_data_);
  out << endl;
  // Read the stats data.
  serial::Serializer::ToJSON(out, key_freq_, params);
}

}  // namespace stats
