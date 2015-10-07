// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_READER_RAW_USER_LOG_READER_H_
#define _PUBLIC_META_LOG_OFFLINE_READER_RAW_USER_LOG_READER_H_

#include "base/defs.h"
#include "meta/log/offline/reader/log_reader.h"

namespace logging {
namespace reader {

// Utility class to iterate over raw user logs.
class RawUserLogReader : public LogReader {
 public:
  using LogReader::LogReader;

  // Parses log files in range [yesterday - num_days, today).
  explicit RawUserLogReader(int num_days) : LogReader(GetRawUserLogsDir(), num_days) {}

  // Parses log files in range [begin_date, end_date). (The expected date format is YYYYMMDD).
  RawUserLogReader(const string& begin_date, const string& end_date)
      : LogReader(GetRawUserLogsDir(), begin_date, end_date) {}

  // Parses log files in range [begin_date, end_date).
  RawUserLogReader(const LocalDate& begin_date, const LocalDate& end_date)
      : LogReader(GetRawUserLogsDir(), begin_date, end_date) {}

  virtual ~RawUserLogReader() = default;

  static const string& GetRawUserLogsDir() {
    static const string dir = "/home/share/data/logs/raw";
    return dir;
  }
};

}  // namespace reader
}  // namespace logging


#endif  // _PUBLIC_META_LOG_OFFLINE_READER_RAW_USER_LOG_READER_H_
