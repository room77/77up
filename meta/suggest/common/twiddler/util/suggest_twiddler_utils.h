// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_COMMON_TWIDDLER_UTIL_SUGGEST_TWIDDLER_UTILS_H_
#define _META_SUGGEST_COMMON_TWIDDLER_UTIL_SUGGEST_TWIDDLER_UTILS_H_

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/common/twiddler/suggest_twiddler.h"

namespace suggest {
namespace twiddle {

// Updates the suggest response completions with the twiddler response.
bool UpdateCompletionsWithTwiddlerResponse(
    const SuggestRequest& suggest_request,
    const SuggestTwiddleResponse& twiddler_response,
    SuggestResponse* suggest_response);

}  // namespace twiddle
}  // namespace suggest


#endif  // _META_SUGGEST_COMMON_TWIDDLER_UTIL_SUGGEST_TWIDDLER_UTILS_H_
