// Copyright 2014 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_META_LOG_OFFLINE_FILTERING_SHARED_BUFFER_WRITER_H_
#define _PUBLIC_META_LOG_OFFLINE_FILTERING_SHARED_BUFFER_WRITER_H_

#include "meta/log/common/log_datatypes.h"
#include "base/defs.h"
#include "util/file/file.h"
#include "util/time/localtime.h"
#include "util/time/timestamp.h"

namespace logging {

// This class has a role of determining
class SharedBufferWriter {
 public:
  explicit SharedBufferWriter(const string& filename) : filename_(filename) {}

  // This method appends an element to the file specified by path
  bool WriteLog(const tLogElement& element);

 private:
  uint64_t last_updated_ = -1;
  string filename_;
  char buffer_[512 * 1024 + 1];
  ofstream file_;
  mutex file_mutex_;  // Mutex to protect multiple threads from writing to file at the same time
};

}  // namespace logging

#endif  // _PUBLIC_META_LOG_OFFLINE_FILTERING_SHARED_BUFFER_WRITER_H_
