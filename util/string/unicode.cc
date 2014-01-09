// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: Uygar Oztekin

// @flags: -licuio -licuuc -licui18n

#include "util/init/init.h"
#include "util/string/unicode.h"

INIT_ADD_REQUIRED("set_locale_to_utf8", -1, []{
  std::setlocale(LC_ALL, "en_US.utf8");
});
