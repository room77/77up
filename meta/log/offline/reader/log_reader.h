// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_READER_LOG_READER_H_
#define _PUBLIC_META_LOG_OFFLINE_READER_LOG_READER_H_

#include <functional>

#include "base/defs.h"
#include "meta/log/offline/reader/log_element_reader.h"
#include "util/time/localtime.h"

namespace logging {
namespace reader {

class LogReader {
 public:
  // Parses log files in range [yesterday - num_days, today).
  // NOTE: Currently the dates for the folders are created using localtime.
  // In case we switch that in future, we may want to switch this too.
  LogReader(const string& dir_name, int num_days)
     : LogReader(dir_name, LocalDate::Today() - num_days, LocalDate::Today()) {}

  // Parses log files in range [begin_date, end_date). (The expected date format is YYYYMMDD).
  LogReader(const string& dir_name, const string& begin_date, const string& end_date)
      : LogReader(dir_name, LocalDate::ParseFromYYYYMMDD(begin_date),
                  LocalDate::ParseFromYYYYMMDD(end_date)) {}

  // Parses log files in range [begin_date, end_date).
  LogReader(const string& dir_name, const LocalDate& begin_date, const LocalDate& end_date)
      : dir_name_(dir_name), begin_date_(begin_date), end_date_(end_date) {}

  virtual ~LogReader() = default;

  // Iterates over all the logs and calls the elem_reader for each line in the logs.
  // Returns the total number of logs processed.
  int ReadLogs(const LogElementReaderInterface& elem_reader) const;

  // Reads a given directory.
  static int ReadDirectory(const string& dirname, const LogElementReaderInterface& elem_reader);

  // Reads a given file.
  static int ReadFile(const string& filename, const LogElementReaderInterface& elem_reader);

  const string& dir_name() const { return dir_name_; }
  const LocalDate& begin_date() const { return begin_date_; }
  const LocalDate& end_date() const { return end_date_; }

 private:
  // Reads a compressed file (.gz).
  static int ReadCompressedFile(const string& filename,
                                const LogElementReaderInterface& elem_reader);

  // Reads an uncompressed file.
  static int ReadUncompressedFile(const string& filename,
                                  const LogElementReaderInterface& elem_reader);

  // The dirname contained dated directories to iterate through.
  string dir_name_;

  // The begin date to read logs from.
  LocalDate begin_date_;

  // The begin date to read logs from.
  LocalDate end_date_;
};

}  // namespace reader
}  // namespace logging

#endif  // _PUBLIC_META_LOG_OFFLINE_READER_LOG_READER_H_
