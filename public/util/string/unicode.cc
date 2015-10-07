// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: Uygar Oztekin

#include "util/init/init.h"
#include "util/string/unicode.h"

INIT_ADD_REQUIRED("set_locale_to_utf8", -1, []{
  std::setlocale(LC_ALL, "en_US.utf8");
});
