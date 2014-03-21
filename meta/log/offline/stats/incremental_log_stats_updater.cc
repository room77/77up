// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/log/offline/stats/incremental_log_stats_updater.h"

#include <chrono>
#include <utility>
#include <limits>

#include "base/defs.h"
#include "util/file/file.h"
#include "meta/log/offline/reader/element_readers/element_reader_lm.h"
#include "meta/log/offline/reader/log_reader.h"
#include "util/serial/serializer.h"
#include "util/time/timestamp.h"

FLAG_string(logging_begin_date, "",
            "The start date from which the logs should be read in format YYYYMMDD. "
            "If this is specified, logs are read starting at this date ending up to "
            "logging_end_date (or today if logging_end_date is not specified");

FLAG_string(logging_end_date, "",
            "The end date to which the logs should be read in format YYYYMMDD. "
            "NOTE: This date is not included.");

FLAG_int(logging_num_days, 180,
         "The number of days going back from today to read logs for.");

FLAG_bool(logging_allow_time_gap, false,
          "Allows a time gap between the stats last read upto date and begin date.");

FLAG_string(logging_log_root_dir, "/home/share/data/logs/raw",
            "The root directory for logs to read from. It is expected the directories/files "
            "are ordered by date under this folder.");

FLAG_string(logging_stats_ids, "",
            "Comma separated list of stat ids that define each stat. Note this must have a 1-1 "
            "mapping with logging_stats_input_files.");

FLAG_string(logging_stats_input_files, "",
            "Comma separated list of files that need to be updated. "
            "These files are expected to be in binary format.");

FLAG_string(logging_stats_output_files, "",
            "Comma separated list of files that need to be written."
            "Note: There must be a 1-1 mapping with logging_stats_input_files.");

FLAG_bool(logging_use_pool, true,
            "Use pool to parse all logs if specified.");

FLAG_bool(logging_stats_output_json_files, false,
          "Set to true if the JSON output file should also be generated.");

FLAG_bool(logging_stats_time_decay_data, true,
          "Time decay data if specified.");


FLAG_double(logging_stats_remove_below_threshold, 0,
          "Remove the keys before the specified threshold.");

namespace logging {
namespace offline {
namespace {

vector<shared_ptr<stats::StatsFile>> GetStatFiles() {
  vector<shared_ptr<stats::StatsFile>> stats_files;

  vector<string> ids;
  strutil::BreakUpString(gFlag_logging_stats_ids, ',', &ids);
  for (const string& id : ids) {
    stats_files.push_back(make_shared<stats::StatsFile>(strutil::Trim(id)));
  }
  return stats_files;
}

bool ParseDates(LocalDate* begin_date, LocalDate* end_date) {
  // Find the end date first.
  if (gFlag_logging_end_date.size()) {
    *end_date = LocalDate::ParseFromYYYYMMDD(gFlag_logging_end_date);
    if (end_date->IsInfinite()) {
      LOG(ERROR) << "Invalid end date: " << gFlag_logging_end_date;
      return false;
    }
  } else {
    // If the end is not set, we set it to today.
    *end_date = LocalDate::Today();
  }

  // Find the begin date.
  if (gFlag_logging_begin_date.size()) {
    *begin_date = LocalDate::ParseFromYYYYMMDD(gFlag_logging_begin_date);
    if (begin_date->IsInfinite()) {
      LOG(ERROR) << "Invalid begin date: " << gFlag_logging_begin_date;
      return false;
    }
  } else {
    if (gFlag_logging_num_days < 1) {
      LOG(ERROR) << "Must specify either --logging_num_days or --logging_start_date";
      return false;
    }
    *begin_date = *end_date - gFlag_logging_num_days;
  }

  if (begin_date->IsInfinite() || end_date->IsInfinite()) {
    LOG(ERROR) << "Dates are infinite.";
    return false;
  }

  if (begin_date > end_date) {
    LOG(ERROR) << "Begin date: " << begin_date->PrintDate_YYYYMMDD() << "cannot be greater than "
               << "end date: " << end_date->PrintDate_YYYYMMDD();
    return false;
  }

  // Both dates are good.
  return true;
}

}  // namespace

int IncrementalLogStatsUpdater::ParseFlags() {
  if (gFlag_logging_log_root_dir.empty()) {
    LOG(ERROR) << "Must specify log root dir!";
    return 1;
  }
  // Get the begin and the end date.
  if (!ParseDates(&begin_date, &end_date)) {
    LOG(ERROR) << "Invalid date parameters: begin [" << gFlag_logging_begin_date << "], end: ["
               << gFlag_logging_end_date << "], days: [" << gFlag_logging_num_days << "]. "
               <<  "Please specify at least --logging_num_days or --logging_start_date";
    return 2;
  }

  // Get the stats file objects.
  stats_files = GetStatFiles();

  // Get the input files.
  strutil::BreakUpString(gFlag_logging_stats_input_files, ',', &input_files);
  if (stats_files.size() != input_files.size() && !input_files.empty()) {
    LOG(ERROR) << "The number of input files ("
               << input_files.size() << ") does not match the number of stat ids("
               << stats_files.size() << ").";
    return 3;
  }


  // Get the output files.
  strutil::BreakUpString(gFlag_logging_stats_output_files, ',', &output_files);
  if (stats_files.size() != output_files.size()) {
    LOG(ERROR) << "The number of output files ("
               << output_files.size() << ") does not match the number of stat ids("
               << stats_files.size() << ").";
    return 4;
  }
  return 0;
}

int IncrementalLogStatsUpdater::Initialize() {
  int status = ParseFlags();
  if (status) return status;

  // Read in all the stats from the existing files.
  // Either these files will be the same size as stats file or there will be no stats files.
  // Also, find the the time from which the logs should really be read from.
  pair<int, uint64_t> read_ahead_from_time(-1, numeric_limits<uint64_t>::max());
  for (int i = 0; i < input_files.size(); ++i) {
    auto& stat = stats_files[i];
    const string& filename = input_files[i];
    if (!stat->ReadBinaryFile(filename)) {
      LOG(ERROR) << "Could not read file: " << filename;
      return 5;
    }
    uint64_t read_upto_time = stat->stats_data()->ReadUptoTimeForSignal("log");
    if (read_upto_time > 0 && read_upto_time < read_ahead_from_time.second)
      read_ahead_from_time = make_pair(i, read_upto_time);
  }

  // Check if the begin time needs to be updated.
  if (read_ahead_from_time.first > -1) {
    LocalDate date = LocalTime::UTCTimeFromTimeStamp(read_ahead_from_time.second);
    if (date > end_date) {
      LOG(INFO) << "The last updated date for any of the stats " << date.PrintDate_YYYYMMDD()
                << " is later than end date: " << end_date.PrintDate_YYYYMMDD();
      return 0;
    }

    if (date < begin_date) {
      if (!gFlag_logging_allow_time_gap) {
        LOG(ERROR) << "The stats need to be updated from: " << date.PrintDate_YYYYMMDD()
                     << " instead of the specified begin date: " << begin_date.PrintDate_YYYYMMDD();
        return 6;
      }
    }

    if (begin_date < date) {
      LOG(INFO) << "The stats only need to be updated from: " << date.PrintDate_YYYYMMDD()
                << " instead of the specified begin date: " << begin_date.PrintDate_YYYYMMDD();
      begin_date = date;
    }
  }

  // Time decay data.
  if (gFlag_logging_stats_time_decay_data) {
    uint64_t now = ::util::Timestamp::PrevMidnightTimeStamp< ::chrono::microseconds>();
    for (auto& stat : stats_files)
      stat->mutable_stats_data()->TimeDecaySignalData("log", now);
  }
  return 0;
}

int IncrementalLogStatsUpdater::ReadLogs() {
  logging::reader::LogReader rd(gFlag_logging_log_root_dir,  begin_date, end_date);
  logging::reader::ReadJSONLogElemFwdToLogManager lm_reader(gFlag_logging_use_pool);
  int processed = rd.ReadLogs(lm_reader);

  // Wait for all logs to have been processed.
  lm_reader.Wait();

  LOG(INFO) << "Read " << processed << " log events.";

  return processed > 0 ? 0 : 21;
}

int IncrementalLogStatsUpdater::PostProcess() {
  if (gFlag_logging_stats_remove_below_threshold > 0) {
    // Remove all keys below the specified threshold.
    for (int i = 0; i < stats_files.size(); ++i) {
      auto& stat = stats_files[i];
      int removed = stat->mutable_stats_data()->RemoveKeysForSignalBelowThreshold(
          "log", gFlag_logging_stats_remove_below_threshold);
      LOG(INFO) << " Removed " << removed << " keys from stat: " << stat->id();
    }
  }
  return 0;
}

int IncrementalLogStatsUpdater::WriteFiles() {
  // Remove all keys below the specified threshold.
  vector<string> failed;
  for (int i = 0; i < stats_files.size(); ++i) {
    auto& stat = stats_files[i];
    const string& out_file = output_files[i];
    if (!stat->WriteBinaryFile(out_file)) failed.push_back(out_file);

    if (gFlag_logging_stats_output_json_files) stat->WriteJSONFile(
        file::ReplaceExtension(out_file, ".json"));
  }

  if (failed.size()) {
    LOG(ERROR) << "Failed to write the following files: " << serial::Serializer::ToJSON(failed);
    return 41;
  }
  return 0;
}

int IncrementalLogStatsUpdater::Update() {
  int status = Initialize();
  if (status) return status;

  // Now that we have the right begin and end date, lets get to work.
  status = ReadLogs();
  if (status) return status;

  // PostProcess the data.
  status = PostProcess();
  if (status) return status;

  // Write the data.
  status = WriteFiles();
  if (status) return status;

  return 0;
}


}  // namespace offline
}  // namespace logging
