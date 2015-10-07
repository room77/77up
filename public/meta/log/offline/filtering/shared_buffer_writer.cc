// Copyright 2014 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/offline/filtering/shared_buffer_writer.h"

#include <mutex>

#include "base/logging.h"

namespace logging {

bool SharedBufferWriter::WriteLog(const tLogElement& element) {
  // VLOG(2) << "Writing " << filename_;

  lock_guard<mutex> l(file_mutex_);

  // If this is the first time writing, we need to create the directory and file
  if (last_updated_ == -1) {
    file::CreateDirectoryIfNecessary(filename_.c_str());
    file_.rdbuf()->pubsetbuf(buffer_, sizeof(buffer_));
    file_.open(filename_.c_str(), ios::app);
    ASSERT(file_.good()) << "File is not good for writing";
  }

  file_ << element.ToJSON();
  file_ << "\n";
  ASSERT(!file_.fail()) << "Error writing to the file";
  last_updated_ = ::util::Timestamp::Now<chrono::seconds>();

  return true;
}

}  // namespace logging

