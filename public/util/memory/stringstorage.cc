#include "util/memory/stringstorage.h"
#include "util/init/init.h"

// global variable to store strings efficiently
StringStorage_CaseInsensitive gStringStorage;

FLAG_bool(stringstorage_lock, true,
          "Set to true if the stringstorage should be locked.");

INIT_ADD_REQUIRED("stringstorage", 99,
    []{ if (gFlag_stringstorage_lock) gStringStorage.Lock(); });
