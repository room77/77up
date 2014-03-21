// Copyright 2014 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "base/defs.h"
#include "meta/log/offline/reader/element_readers/element_reader_lm.h"
#include "meta/log/offline/reader/log_reader.h"
#include "util/time/localtime.h"

FLAG_string(logging_log_root_dir, "/home/share/data/logs/raw",
            "The root directory for logs to read from. It is expected the directories/files "
            "are ordered by date under this folder.");

FLAG_bool(logging_use_pool, true, "Use pool to parse all logs if specified.");

FLAG_string(logging_begin_date, "20140101",
            "The start date from which the logs should be read in format YYYYMMDD. "
            "If this is specified, logs are read starting at this date ending up to "
            "logging_end_date (or today if logging_end_date is not specified");

FLAG_string(logging_end_date, "",
            "The end date to which the logs should be read in format YYYYMMDD. "
            "NOTE: This date is not included.");

int init_main() {
  LOG(INFO) << "Running the filtering controller!";

  LocalDate begin_date, end_date;
  // Find the end date first.
  if (gFlag_logging_end_date.size()) {
    end_date = LocalDate::ParseFromYYYYMMDD(gFlag_logging_end_date);
    if (end_date.IsInfinite()) {
      LOG(ERROR) << "Invalid end date: " << gFlag_logging_end_date;
      return -1;
    }
  } else {
    // If the end is not set, we set it to today.
    end_date = LocalDate::Today();
  }

  // Find the begin date.
  if (gFlag_logging_begin_date.size()) {
    begin_date = LocalDate::ParseFromYYYYMMDD(gFlag_logging_begin_date);
    if (begin_date.IsInfinite()) {
      LOG(ERROR) << "Invalid begin date: " << gFlag_logging_begin_date;
      return -1;
    }
  } else {
    LOG(ERROR) << "Begin date must be set";
    return -1;
  }

  if (begin_date >= end_date) {
    LOG(ERROR) << "Begin date needs to be smaller than end_date";
    return -1;
  }

  logging::reader::LogReader rd(gFlag_logging_log_root_dir,  begin_date, end_date);
  logging::reader::ReadJSONLogElemFwdToLogManager log_filter_reader;
  int processed = rd.ReadLogs(log_filter_reader);

  // Wait for all logs to have been processed.
  log_filter_reader.Wait();

  LOG(INFO) << "Read " << processed << " log events.";

  return processed > 0 ? 0 : 21;
}
