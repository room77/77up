// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_READER_ELEMENT_READERS_ELEMENT_READER_LM_H_
#define _PUBLIC_META_LOG_OFFLINE_READER_ELEMENT_READERS_ELEMENT_READER_LM_H_

#include <memory>

#include "base/defs.h"

#include "meta/log/common/log_datatypes.h"
#include "meta/log/offline/reader/log_element_reader.h"

namespace logging {
namespace reader {

struct ReadLogElemFwdToLogManagerBase : public LogElementReaderInterface {
  explicit ReadLogElemFwdToLogManagerBase(const bool async = true) : async(async) {}

  virtual ~ReadLogElemFwdToLogManagerBase() = default;

  // The function called for each element.
  virtual int Read(const string& element, const string& filename) const = 0;

  // Wait for all the logs to have been processed.
  virtual void Wait() const;

 protected:
  // Forwards a message to the log manager.
  void ForwardElementToLogManager(const shared_ptr<tLogElement>& elem,
                                  const string& parser_type) const;

  // By default all calls to the log manager are asynchronous.
  bool async = true;
};

// Simple reader that takes in a single JSON line and passes it on to log manager.
// The assumption is there would be appropriate triggers registered with the manager that can
// handle corresponding events.
struct ReadJSONLogElemFwdToLogManager : public ReadLogElemFwdToLogManagerBase {
  using ReadLogElemFwdToLogManagerBase::ReadLogElemFwdToLogManagerBase;

  virtual ~ReadJSONLogElemFwdToLogManager() = default;

  // The function called for each element.
  virtual int Read(const string& element, const string& filename) const;
};

// Simple reader that takes in a single JSON line and passes it on to log manager.
// The assumption is there would be appropriate triggers registered with the manager that can
// handle corresponding events.
struct ReadBinaryLogElemFwdToLogManager : public ReadLogElemFwdToLogManagerBase {
  using ReadLogElemFwdToLogManagerBase::ReadLogElemFwdToLogManagerBase;

  virtual ~ReadBinaryLogElemFwdToLogManager() = default;

  // The function called for each element.
  virtual int Read(const string& element, const string& filename) const;
};

}  // namespace reader
}  // namespace logging


#endif  // _PUBLIC_META_LOG_OFFLINE_READER_ELEMENT_READERS_ELEMENT_READER_LM_H_
