// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_METHODS_SUGGEST_METHOD_UTILS_H_
#define _META_SUGGEST_SERVER_METHODS_SUGGEST_METHOD_UTILS_H_

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "meta/suggest/server/methods/suggest_methods.h"

namespace suggest {
namespace methods {

// Fixes the parent suggestion.
void FixParentSuggestion(const SuggestRequestInterface& req, int name_count,
                         GetSuggestions::ReleaseReply::CompleteSuggestionReply* reply);

// Fixes the child suggestion.
void FixChildSuggestion(const Completion& child, const Completion& parent,
                        GetSuggestions::ReleaseReply::CompleteSuggestionReply* reply);

}  // namespace methods
}  // namespace suggest


#endif  // _META_SUGGEST_SERVER_METHODS_SUGGEST_METHOD_UTILS_H_
