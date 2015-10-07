// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_UTILS_LOCATION_UTILS_H_
#define _PUBLIC_META_LOG_OFFLINE_UTILS_LOCATION_UTILS_H_

#include "base/defs.h"
#include "meta/log/common/log_datatypes.h"

namespace logging {
namespace offline {
namespace utils {

vector<string> GetLocationIdsFromSgstId(const string& suggest_id);

// Returns the location ids associated with the element.
// We first look for the 'sgst_id' field. If it is available, we parse locations from the sgst_id.
// If the 'sgst_id' is not available, then we look at the 'key' in cgi paramters.
// If the key is not empty but we failed to parse it, and fill_key_if_none_found is set to true
// the key is returned as the id.
vector<string> GetLocationIds(const tLogElement& element, bool fill_key_if_none_found = true);

}  // namespace utils
}  // namespace offline
}  // namespace logging


#endif  // _PUBLIC_META_LOG_OFFLINE_UTILS_LOCATION_UTILS_H_
