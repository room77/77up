// Copyright 2007 OpTrip, Inc.
//
// Streamlined version for open sourcing.

#ifndef _PUBLIC_BASE_DEFS_H_
#define _PUBLIC_BASE_DEFS_H_

#include "base/disable_unsafe.h"
#include "util/hash/std_hash_extra.h"

using namespace std;

// Let's define int128_t and uint128_t until GCC adds them.
typedef __int128_t  int128_t;
typedef __uint128_t uint128_t;

#endif  // _PUBLIC_BASE_DEFS_H_
