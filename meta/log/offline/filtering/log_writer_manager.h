// Copyright 2014 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_OFFLINE_FILTERING_LOG_WRITER_MANAGER_H_
#define _PUBLIC_META_LOG_OFFLINE_FILTERING_LOG_WRITER_MANAGER_H_

#include <memory>
#include <unordered_map>

#include "base/defs.h"
#include "meta/log/common/log_datatypes.h"
#include "meta/log/offline/filtering/shared_buffer_writer.h"

namespace logging {

// This class is a singleton through which all the write requests are processed
class LogWriterManager {
 public:
  virtual ~LogWriterManager() {}

  static LogWriterManager& Instance() {
    static LogWriterManager the_one;
    return the_one;
  }

  // This method takes in a single log element and adds it to the proper file buffer for writing
  void WriteLogElement(const tLogElement& element, const string& filename);

  // This method wakes up every interval seconds and closes all the files that haven't been touched
  // during the last timeout.
  // TODO(otasevic): implement this method
  void CloseFiles();

 protected:
  LogWriterManager() = default;

  typedef unordered_map<string, shared_ptr<SharedBufferWriter> > WritersMap;

 private:
  WritersMap writers_map_;  // Holds a mapping from filename to an instance of SharedBufferWriter
  mutex writers_map_mutex_;  // The mutex to protect writing to writers_map_
};

}  // namespace logging

#endif  // _PUBLIC_META_LOG_OFFLINE_FILTERING_LOG_WRITER_MANAGER_H_
