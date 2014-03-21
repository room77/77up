// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/stats/signal_stats.h"

#include "util/time/decay/time_decay.h"

namespace stats {

uint64_t SignalStats::ReadUptoTimeForSignal(const string& signal) const {
  lock_guard<std::mutex> l(m_);
  const BasicMetaData& md = meta_data(signal);

  if (md.read_ahead_from_time == 0) return md.timestamp;
  return md.read_ahead_from_time;
}

bool SignalStats::TimeDecaySignalData(const string& signal, uint64_t now) {
  // Decay all values by the amount.
  lock_guard<std::mutex> l(m_);
  BasicMetaData& md = meta_data_[signal];
  if (now < md.timestamp) return false;

  md.read_ahead_from_time = md.timestamp;
  md.timestamp = now;

  // Setup the decay object.
  // We assume here, that even if this function is called multiple times, they will all have the
  // same now.
  SetupTimeDecay(now);

  // All the new data added with now will be associated with this timestamp.
  md.timestamp = now;
  // Check if there are any value associated with the signal.
  if (md.n == 0) return true;

  // Clear the meta data.
  md.clear();

  // Decay all the previously recorded values appropriately.
  double decay_amount = GetDecayAmount(md.read_ahead_from_time);
  for (auto& p : key_freq_) {
    auto iter = p.second.find(signal);
    if (iter == p.second.end()) continue;
    iter->second *= decay_amount;
    md.Increment(iter->second);
  }
  return true;
}

void SignalStats::IncrementKeyForSignal(const string& signal, const string& key, double value,
                                        bool update_metadata) {
  lock_guard<std::mutex> l(m_);
  key_freq_[key][signal] += value;
  if (update_metadata) meta_data_[signal].Increment(value);
}

void SignalStats::IncrementKeyForSignalWithTimestamp(const string& signal, const string& key,
                                                     uint64_t timestamp, double value,
                                                     bool update_metadata) {
  // NOTE(pramodg): This is currently not locked for optimization.
  // The expectation is that IncrementKeyForSignal will not be called in parallel with
  // TimeDecaySignalData. If, however, there is a client that has a valid need for doing this,
  // this part needs to be locked too.
  auto iter = meta_data_.find(signal);
  // Check if the value is older than what we want to read from.
  if (iter != meta_data_.end() && timestamp < iter->second.read_ahead_from_time) return;

  // Decay the value appropriately based on the timestamp.
  value *= GetDecayAmount(timestamp);

  // Increment the key with the value.
  IncrementKeyForSignal(signal, key, value, update_metadata);
}

// Increment the key.
double SignalStats::GetKeyForSignal(const string& signal, const string& key) const {
  static const map<string, double> kDefaultSignalMap;
  lock_guard<std::mutex> l(m_);
  return ::util::tl::FindWithDefault(::util::tl::FindWithDefault(
      unsafe_key_signal_freq_map(), key, kDefaultSignalMap), signal, 0);
}


// Removes keys that are below the given threshold.
int SignalStats::RemoveKeysForSignalBelowThreshold(const string& signal, double threshold) {
  lock_guard<std::mutex> l(m_);
  BasicMetaData& md = meta_data_[signal];

  // If there are no values associated with the signal, nothing will be deleted.
  if (md.n == 0) return 0;

  md.clear();
  int count = 0;
  for (auto iter = key_freq_.begin(); iter != key_freq_.end();) {
    auto sig_iter = iter->second.find(signal);
    if (sig_iter == iter->second.end()) {
      ++iter;
      continue;
    }

    if (sig_iter->second < threshold) {
      iter->second.erase(sig_iter);

      // Check if there are any more signals associated with the key.
      // If there are none, remove the key.
      if (iter->second.empty()) iter = key_freq_.erase(iter);
      else ++iter;

      ++count;
      continue;
    }
    md.Increment(sig_iter->second);
    ++iter;
  }
  return count;
}

// Serialization interface.
bool SignalStats::FromBinary(istream& in) {
  lock_guard<std::mutex> l(m_);
  // Read the first metadata line.
  if (!serial::Serializer::FromBinary(in, &meta_data_)) return false;

  // Read the stats data.
  return serial::Serializer::FromBinary(in, &key_freq_);
}

bool SignalStats::FromJSON(istream& in) {
  lock_guard<std::mutex> l(m_);
  // Read the first metadata line.
  if (!serial::Serializer::FromJSON(in, &meta_data_)) return false;

  // Read the stats data.
  return serial::Serializer::FromJSON(in, &key_freq_);
}

void SignalStats::ToBinary(ostream& out) const {
  lock_guard<std::mutex> l(m_);

  // Read the first metadata line.
  serial::Serializer::ToBinary(out, meta_data_);
  // Read the stats data.
  serial::Serializer::ToBinary(out, key_freq_);
}

void SignalStats::ToJSON(ostream& out, const serial::JSONSerializationParams& params) const {
  lock_guard<std::mutex> l(m_);
  // Read the first metadata line.
  serial::Serializer::ToJSON(out, meta_data_);
  out << endl;
  // Read the stats data.
  serial::Serializer::ToJSON(out, key_freq_, params);
}

}  // namespace stats
