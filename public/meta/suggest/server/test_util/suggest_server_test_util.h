// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_TEST_UTIL_SUGGEST_SERVER_TEST_UTIL_H_
#define _META_SUGGEST_SERVER_TEST_UTIL_SUGGEST_SERVER_TEST_UTIL_H_

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"

namespace suggest {
namespace test {

// Returns a mock suggest request with basic params filled.
SuggestRequest MakeMockSuggestRequest(const string& norm_query = "test");

// Returns a mock CompleteSuggestion with basic params filled.
CompleteSuggestion MakeMockCompleteSuggestion(double score = 100,
                                              const string& norm = "test");

// Returns a mock Completion with basic params filled.
Completion MakeMockCompletion(double score = 100,
                              const string& norm = "test");

// Returns a mock SuggestResponse with n completions.
// The names of the completions are {id0, id1, ... idn}.
SuggestResponse MakeMockSuggestResponse(const string& id, double base_score,
                                        int n);

}  // namespace test
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_TEST_UTIL_SUGGEST_SERVER_TEST_UTIL_H_
