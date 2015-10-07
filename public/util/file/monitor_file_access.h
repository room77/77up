// Copyright 2013 Room77, Inc.
// Author: B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_FILE_MONITOR_FILE_ACCESS_H_
#define _PUBLIC_UTIL_FILE_MONITOR_FILE_ACCESS_H_

#include "base/logging.h"

extern bool gFlag_record_file_access;

// assert unless running in record file access mode
// use this for any assert related to reading data from files
#define ASSERT_IGNORE_IN_RECORD_FILE_ACCESS_MODE(expr) \
  gFlag_record_file_access ? \
  LOG_IF(ERROR, !(expr)) << "ASSERT failed: " #expr ": " << (expr) : \
  ASSERT(expr) << "ASSERT failed: " #expr ": " << (expr)

#endif
