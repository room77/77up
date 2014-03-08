#include "util/memory/stringstorage.h"
#include "test/cc/test_main.h"
#include "util/init/init.h"

// Unlock stringstorage for testing purposes. This should never be done in
// production in a multi-threaded environment. Stringstroge is NOT thread safe.

extern bool gFlag_stringstorage_lock;
INIT_ADD_REQUIRED("stringstorage_test", 90, []{ gFlag_stringstorage_lock = false; });
