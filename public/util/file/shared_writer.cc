// Copyright 2012 Room77, Inc.
// Author: Nicholas Edelman

#include "util/file/shared_writer.h"

FLAG_int(shared_writer_sleep_ms, 100, 
         "the number of milliseconds to sleep while waiting for the next results");

namespace util {

SharedWriter::SharedWriter(const string& fn) : fn_(fn) {
  file_.open(fn, ios::out);
  ASSERT(file_.good()) << "Error unable to open for writing: " << fn;
  thread(&util::SharedWriter::Flush, this).detach();
}

void SharedWriter::Write(const string& entry) {
  lock_guard<mutex> lg(m_);
  entries_.push_back(entry);
}

void SharedWriter::Reset() {
  lock_guard<mutex> lg(m_);
  entries_.clear();
  file_.close();
  file_.open(fn_, ios::out);
  ASSERT(file_.good()) << "Error unable to REopen for writing: " << fn_;
}


//
// PRIVATE
//

void SharedWriter::Flush() {
  while (true) {
    vector<string> entries;
    GetEntries(&entries);
    if (entries.size() > 0) {
      for (auto it = entries.begin(); it != entries.end(); ++it) {
        file_ << *it << endl;
      }
      file_.flush();
      ASSERT(!file_.fail()) << "Error flushing " << entries.size() 
                            << " entries to file" << fn_;
    } else {
      this_thread::sleep_for(chrono::milliseconds(gFlag_shared_writer_sleep_ms));
    }
  }
}

void SharedWriter::GetEntries(vector<string> *entries) {
  lock_guard<mutex> lg(m_);
  entries->clear();
  entries->swap(entries_);
}


} // namespace util
