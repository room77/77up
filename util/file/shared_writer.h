// Copyright 2012 Room77, Inc.
// Author: Nicholas Edelman

//
// Simple threadsafe file writer that flushes files to disk
//

#ifndef _PUBLIC_UTIL_FILE_SHARED_WRITER_H_
#define _PUBLIC_UTIL_FILE_SHARED_WRITER_H_

#include "base/common.h"

#include <fstream>
#include <mutex>
#include <thread>

namespace util {

class SharedWriter {
 public:
  // @param fn - the path to the file
  SharedWriter(const string& fn);
  ~SharedWriter() {}
  // queues up the entry to be written
  // @threadsafe
  void Write(const string& entry);
  // flushes the queue and restarts the write at the beginning of the file
  void Reset();
 private:
  // flush the entries to disk. runs the main flushing thread.
  // this is the ONLY place where things are written
  // flushes after every write
  void Flush();
  // copy the current entries in the vector
  // @threadsafe
  void GetEntries(vector<string> *entries);

  // the mutex to protect the entries
  std::mutex m_;
  // the file to write to
  ofstream file_;
  // the entries to flush to disk
  vector<string> entries_;
  // store the fn
  const string fn_;
};

} // namespace util

#endif  // _PUBLIC_UTIL_FILE_SHARED_WRITER_H_
