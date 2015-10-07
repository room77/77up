// Copyright 2013 B. Uygar Oztekin

#ifndef _PUBLIC_UTIL_STORE_TEST_TEST_MACROS_H_
#define _PUBLIC_UTIL_STORE_TEST_TEST_MACROS_H_

#include "util/store/test/test.h"

// Hijack the assert macro for store testing purposes.
// This removes dependency on r77 libraries for most store implementations.
// They can be made open source easier that way.

// This assert macro aborts the current test only, it does not abort the program.
#undef ASSERT
#define ASSERT(X) if (!(X)) { cout << '\n' << __FILE__ << ":" << __LINE__ << \
    ": Assertion failed: " << #X << endl; return false; }

#endif
