// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_UTIL_SUGGEST_UTILS_H_
#define _META_SUGGEST_UTIL_SUGGEST_UTILS_H_

#include "base/defs.h"
#include "meta/suggest/common/suggest_datatypes.h"

namespace suggest {
namespace util {

// Returns the name for the algo type.
const string GetAlgoNameFromType(const SugggestionAlgoType& type);

}  // namespace util
}  // namespace suggest


#endif  // _META_SUGGEST_UTIL_SUGGEST_UTILS_H_
