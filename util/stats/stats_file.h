// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_STATS_STATS_FILE_H_
#define _UTIL_STATS_STATS_FILE_H_

#include <fstream>

#include "base/defs.h"
#include "util/file/file.h"
#include "base/logging.h"
#include "util/stats/stats.h"
#include "util/serial/serializer.h"
#include "util/time/localtime.h"

namespace stats {

// A basic stats file.
// This file maintains the different stats.
class StatsFile {
 public:
  explicit StatsFile(const string& stat_id) : id_(stat_id) {
    stats_data_ =  StatsInterface::make_shared(stat_id);
    ASSERT_NOTNULL(stats_data_) << "Invalid id: " << stat_id;
  }

  bool ReadFile(const string& filename, const string& format = "binary") {
    VLOG(2) << "Reading " << filename;
    if (!file::IsFile(filename)) {
      VLOG(2) << "Could not find: " << filename;
      return false;
    }

    ifstream f(filename.c_str());
    if (!f.good()) {
      VLOG(2) << "Invalid file data: " << filename;
      return false;
    }

    bool res = false;
    if (format == "binary") res = FromBinary(f);
    else if (format == "json") res = FromJSON(f);
    return res;
  }

  bool ReadBinaryFile(const string& filename) {
    return ReadFile(filename, "binary");
  }

  bool ReadJSONFile(const string& filename) {
    return ReadFile(filename, "json");
  }

  // Writes the data to the file.
  bool WriteFile(const string& filename, const string& format = "binary") {
    VLOG(2) << "Writing " << filename;
    if (file::IsFile(filename)) {
      VLOG(2) << "Overwriting: " << filename;
    }

    // Create directories as needed.
    file::CreateDirectoryIfNecessary(filename.c_str());

    ofstream f(filename.c_str());
    if (!f.good()) {
      VLOG(2) << "Could not open file to write: " << filename;
      return false;
    }

    if (format == "binary") ToBinary(f);
    else if (format == "json") ToJSON(f);

    return true;
  }

  bool WriteBinaryFile(const string& filename) {
    return WriteFile(filename, "binary");
  }

  bool WriteJSONFile(const string& filename) {
    return WriteFile(filename, "json");
  }

  const string& id() const { return id_; }

  // Returns the last updated time for this file.
  const LocalDate& LastUpdated() const { return meta_data_.last_updated; }

  // Set the last updated time for this file.
  void SetLastUpdated(const LocalDate& date) { meta_data_.last_updated = date; }

  // Returns the meta data associated with the file.
  const StatsInterface::shared_proxy& stats_data() const { return stats_data_;}
  StatsInterface::mutable_shared_proxy& mutable_stats_data() { return stats_data_;}

  bool FromBinary(istream& in) {
    // Read the first metadata line.
    if (!serial::Serializer::FromBinary(in, &meta_data_)) return false;

    // Read the stats data.
    if (!stats_data_->FromBinary(in)) return false;
    return true;
  }

  bool FromJSON(istream& in) {
    // Read the first metadata line.
    if (!serial::Serializer::FromJSON(in, &meta_data_)) return false;

    // Read the stats data.
    if (!stats_data_->FromJSON(in)) return false;
    return true;
  }

  bool ToBinary(ostream& out) {
    // Write the first metadata line.
    serial::Serializer::ToBinary(out, meta_data_);

    // Write the stats data.
    stats_data_->ToBinary(out);
    return true;
  }

  bool ToJSON(ostream& out) {
    // Write the first metadata line.
    serial::Serializer::ToJSON(out, meta_data_, {1, 1});

    // Write the stats data.
    stats_data_->ToJSON(out, {1, 1});
    return true;
  }

 protected:
  struct FileMetaData {
    // The date of the latest log file used to create the stats.
    LocalDate last_updated = LocalDate::Today();

    SERIALIZE(DEFAULT_CUSTOM / last_updated*1);
  };

 private:
  string id_;

  // The full metadata associated with the file.
  FileMetaData meta_data_;

  // The stats data associated with the file.
  StatsInterface::mutable_shared_proxy stats_data_;
};

}  // namespace stats


#endif  // _UTIL_STATS_STATS_FILE_H_
