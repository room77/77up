// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "test/cc/test_main.h"

#include "util/file/file.h"
#include "util/init/init.h"

FLAG_string(test_dir, "/tmp/r77_test_dir.XXXXXXXX",
            "The test dir available to each test for temporary file writes. "
            "This directory is cleaned up after the test finishes.");

extern bool gFlag_stringstorage_lock;

INIT_ADD_REQUIRED("test", 0, []{
   gFlag_test_dir = file::MakeTempDir("/tmp/r77_test_dir.XXXXXXXX");
   LOG(INFO) << "Test: " << gFlag_test_dir;
});

#ifdef R77_USE_STRINGSTORAGE
INIT_ADD_REQUIRED("test", 90, []{ gFlag_stringstorage_lock = false; });
#endif
