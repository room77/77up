// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/log/offline/reader/log_reader.h"

#include <functional>
#include <future>

#include "base/defs.h"
#include "util/file/file.h"
#include "util/string/strutil.h"
#include "util/compress/gzip.h"
#include "util/file/filereader.h"
#include "util/time/simple_timer.h"

FLAG_string(compression_ext, ".gz",
            "The extension for compressed files.");

FLAG_int(max_async_calls, 20, "");

namespace logging {
namespace reader {

int LogReader::ReadLogs(const LogElementReaderInterface& elem_reader) const {
  if (!(begin_date_ <  end_date_)) {
    LOG(ERROR) << "Begin date " << begin_date().PrintFormatted("%Y%m%d") << " must be less than "
        << end_date().PrintFormatted("%Y%m%d");
    return -1;
  }

  LOG(INFO) << "Reading dates: " << begin_date().PrintFormatted("%Y%m%d") << " to "
            << end_date().PrintFormatted("%Y%m%d") << " in directory: " << dir_name();

  ::util::time::ScopedMinutesTimer timer("LOG Time: (mins):");

  vector<future<int>> counts;
  int finished_so_far = 0;
  int num_read = 0;
  counts.reserve(end_date() - begin_date());
  for (LocalDate date = begin_date(); date != end_date(); ++date) {
    const string date_path = file::JoinPath(dir_name(), date.PrintFormatted("%Y%m%d"));
    if (file::IsDirectory(date_path)) {
      // A directory.
      counts.push_back(std::async(
          std::launch::async, &LogReader::ReadDirectory, date_path, std::ref(elem_reader)));
    } else if (file::IsFile(date_path)) {
      // A simple file.
      counts.push_back(std::async(
          std::launch::async, &LogReader::ReadUncompressedFile, date_path, std::ref(elem_reader)));
    } else {
      // Check if we can find a compressed file for the name.
      string gzipped_path = date_path + gFlag_compression_ext;
      if (file::IsFile(gzipped_path))
        counts.push_back(std::async(std::launch::async,
            &LogReader::ReadCompressedFile, gzipped_path, std::ref(elem_reader)));
      else  // Could not find anything with the path.
        LOG(WARNING) << "Could not read: " << date_path;
    }
    // Wait on the older calls if we have too many outstanding async calls.
    for (; counts.size() - finished_so_far > gFlag_max_async_calls;
        num_read += counts[finished_so_far++].get()) ;  // no op
  }

  for (; finished_so_far < counts.size();
      num_read += counts[finished_so_far++].get()) ; // no op
  return num_read;
}

int LogReader::ReadDirectory(const string& dirname, const LogElementReaderInterface& elem_reader) {
  if (dirname.empty() || !file::IsDirectory(dirname)) return -1;

  vector<string> files;
  if (!file::GetFileNames(dirname, &files)) {
    LOG(WARNING) << "No file in directory: " << dirname;
    return 0;
  }

  VLOG(3) << "Reading directory: " << dirname;
  vector<future<int>> counts;
  counts.reserve(files.size());
  int finished_so_far = 0;
  int num_read = 0;
  for (const string& file : files) {
    counts.push_back(std::async(std::launch::async, &LogReader::ReadFile, file,
                                std::ref(elem_reader)));
    for (; counts.size() - finished_so_far > gFlag_max_async_calls;
        num_read += counts[finished_so_far++].get()) ;  // no op
  }

  for (; finished_so_far < counts.size();
      num_read += counts[finished_so_far++].get()) ; // no op
  return num_read;
}

int LogReader::ReadFile(const string& filename, const LogElementReaderInterface& elem_reader) {
  if (filename.empty() || !file::IsFile(filename)) return -1;

  if (file::HasExtension(filename, gFlag_compression_ext))
    return ReadCompressedFile(filename, elem_reader);

  return ReadUncompressedFile(filename, elem_reader);
}

int LogReader::ReadCompressedFile(const string& filename,
                                  const LogElementReaderInterface& elem_reader) {
  VLOG(4) << "Reading compressed file: " << filename;
  auto func = [&](const string& element) -> int { return elem_reader.Read(element, filename); };
  return Compression::ProcessGzipLines(filename, func);
}

int LogReader::ReadUncompressedFile(const string& filename,
                                    const LogElementReaderInterface& elem_reader) {
  // TODO(pramodg): This does not really support multiline stuff.
  // Fix this once there is a need for it.
  VLOG(4) << "Reading uncompressed file: " << filename;
  auto func = [&](const string& element) -> int { return elem_reader.Read(element, filename); };
  return FileReader::ProcessAll<string>(filename, func, false, FileReader::StringLineParser());
}

}  // namespace reader
}  // namespace logging
