// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_MOCK_USER_LOG_ELEMENT_MOCK_H_
#define _PUBLIC_META_LOG_OFFLINE_MOCK_USER_LOG_ELEMENT_MOCK_H_

#include "base/defs.h"
#include "meta/log/offline/user_log_element.h"

namespace logging {
namespace test {

tUserLogReadBackElement MakeMockUserLogReadBackElement(const string& category,
                                                       const string& action);

}  // namespace test
}  // namespace logging


#endif  // _PUBLIC_META_LOG_OFFLINE_MOCK_USER_LOG_ELEMENT_MOCK_H_
