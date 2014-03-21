// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)


#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_APPLICATION_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_APPLICATION_H_

#include "base/defs.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

////////////////////////////////////////////////////////////
// EVENT STRUCTS
////////////////////////////////////////////////////////////

struct tEventApplicationNewVisit {
  string referrer = "";

  SERIALIZE(SERIALIZE_REQUIRED / referrer*1);
};

struct tEventApplicationUncaughtException {
  string message = "";
  string url = "";
  int line_num = 0;

  SERIALIZE(SERIALIZE_OPTIONAL / message*1 / url*2 / line_num*3);
};

struct tEventApplicationAngularException {
  string stack = "";
  string message = "";
  string value = "";

  SERIALIZE(SERIALIZE_OPTIONAL / stack*1 / message*2 / value*3);
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_APPLICATION_H_
