// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_READER_LOG_ELEMENT_READER_H_
#define _PUBLIC_META_LOG_OFFLINE_READER_LOG_ELEMENT_READER_H_

#include "base/defs.h"

namespace logging {
namespace reader {

// Interface for reading a single element of a log file.
// This could be as simple as a single line representing JSON blob or bunch of lines
// representing a binary blob.
struct LogElementReaderInterface {
  virtual ~LogElementReaderInterface() = default;

  // The function called for each element.
  // The return value can be either fo.
  // +ve : The line was read successfully and we should continue to the next line.
  // 0 : There was error parsing the line. The file should not be processed any further.
  // -ve : The line does not constitute the complete element. We need to read more lines. This
  //       allows us to have a single element in multiple lines.
  virtual int Read(const string& element, const string& filename) const = 0;
};

// Utility struct to all subclasses to maintain data that can be modified with each read call.
struct LogElementMutableReaderInterface : public LogElementReaderInterface {
  virtual ~LogElementMutableReaderInterface() = default;

  // The function called for each element.
  virtual int Read(const string& element, const string& filename) const {
    return const_cast<LogElementMutableReaderInterface*>(this)->ReadMutable(element);
  }

  // The function called for each element. This function can modify contents.
  // Subclasses must ensure that this function is thread safe.
  virtual int ReadMutable(const string& element) = 0;
};

}  // namespace reader
}  // namespace logging

#endif  // _PUBLIC_META_LOG_OFFLINE_READER_LOG_ELEMENT_READER_H_
