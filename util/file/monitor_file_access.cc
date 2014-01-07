// Copyright 2013 Room77, Inc.
// Author: B. Uygar Oztekin

// Intercept fopen64 calls so that we can support recording file accesses and
// transparent init data directory overrides. For now, let's replace fopen64
// only. Our codebase does not seem to use others yet (fopen, open, open64).

#include <unistd.h>
#include "base/args/args.h"
#include "base/logging.h"
#include "util/init/init.h"

FLAG_bool(record_file_access, false,
    "If set, record file accesses to gFlag_record_file_access_output.");
FLAG_string(record_file_access_output, "/dev/stderr",
    "See record_file_access.");
FLAG_string(init_data_dir, "",
    "If set, try this directory first for all file accesses.");

#include <dlfcn.h>
#define _FCNTL_H
#include <bits/fcntl.h>

extern string gFlag_init_data_dir;

FILE* fopen64(const char* filename, const char* mode) {
  struct Open {
    FILE* operator()(const char* filename, const char* mode) {
      // Cache the original version at first call (thread safe).
      static FILE* (*_fopen64) (const char *, const char *) =
          (FILE * (*)(const char *, const char *))dlsym(RTLD_NEXT, "fopen64");
      FILE* f = _fopen64(filename, mode);
      // If set, record file accesses.
      if (f && gFlag_record_file_access) {
        FILE* log = _fopen64(gFlag_record_file_access_output.c_str(), "a");
        ASSERT(log) << "Could not open: " << gFlag_record_file_access_output;
        fprintf(log, "%s\n", filename);
        fclose(log);
      }
      return f;
    }
  };

  FILE *f;
  if (!gFlag_init_data_dir.empty()) {
    f = Open()((gFlag_init_data_dir + filename).c_str(), mode);
    if (f) {
      VLOG(2) << "Using: " << gFlag_init_data_dir + filename;
      return f;
    }
  }
  f = Open()(filename, mode);
  return f;
}

// We do not want to incur extra file open overhead after initializations.
// Disable init data dir at the end of all initializations.
INIT_ADD_REQUIRED("disable_init_data_dir", 1000,
                  []{ gFlag_init_data_dir.clear(); });

// Force quit at the end of all exit hooks in record file access mode.
// Some of are binaries may stay alive forever or for a long time. Or some may
// not have a graceful way to terminate fast (complicated thread pool
// situations). Force quit the binary to take care of the above cases.
EXIT_ADD("file_exit", 1000, []{
  if (gFlag_record_file_access) {
    LOG(INFO) << "File access recording mode is enabled. Exiting.";
    _Exit(0);
  }
});
