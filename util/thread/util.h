// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// File for miscellaneous threading utility functions.

#ifndef _PUBLIC_UTIL_THREAD_UTIL_H_
#define _PUBLIC_UTIL_THREAD_UTIL_H_

#include <sstream>
#include <vector>

#include "base/common.h"
#include "base/file.h"
#include "base/strutil.h"

namespace util {
namespace threading {
namespace util {

// Returns all thread ids in the process.
template<typename C>
static C GetAllThreadIds() {
  C res;
  vector<string> task_paths;
  file::GetMatchingFiles("/proc/self/task/*", &task_paths);

  for (const string& path : task_paths) {
    string id = strutil::ReplaceAll(path, "/proc/self/task/", "");
    VLOG(3) << path << " -> " << id;
    stringstream strm(id);
    int tid = 0;
    strm >> tid;
    res.push_back(tid);
  }
  return res;
}

}  // namespace util
}  // namespace threading
}  // namespace util



#endif  // _PUBLIC_UTIL_THREAD_UTIL_H_
