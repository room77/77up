// Copyright 2014 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/offline/filtering/log_writer_manager.h"

#include <mutex>

namespace logging {

void LogWriterManager::WriteLogElement(const tLogElement& element,
                                       const string& filename) {
  WritersMap::iterator it;
  {
    lock_guard<mutex> l(writers_map_mutex_);
    // look at whether the file was created before
    it = writers_map_.find(filename);
    if (it == writers_map_.end()) {  // if filename not found insert into the map
      it = (writers_map_.insert({filename,
          shared_ptr<SharedBufferWriter>(new SharedBufferWriter(filename))})).first;
    }
  }
  it->second->WriteLog(element);
}

}  // namespace logging

