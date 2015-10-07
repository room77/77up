// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

// this file contains all the events associated
// with categories that have a single
// corresponding action...

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_MISCELLANEOUS_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_MISCELLANEOUS_H_

#include "base/common.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

////////////////////////////////////////////////////////////
// EVENT STRUCTS
////////////////////////////////////////////////////////////

// TODO(otasevic) modify this struct
// represents the underlying json blob inside the request.value
// when the action Home Experiment occcurs
struct tEventHomeExperiment {
  string index;

  SERIALIZE(index*1);
};

// represents the underlying json blob inside the request.value
// when the event "New Visit" occcurs
struct tEventNewVisit {
  string value;  // this event does not have any data

  SERIALIZE(DEFAULT_CUSTOM / value*1);
};

// represents the underlying json blob inside the request.value
// when the event "Page Exit" occcurs
struct tEventPageExit {
  uint64_t duration = 0;  // duration in ms
  string page;

  SERIALIZE(DEFAULT_CUSTOM / duration*1 / page*2);
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_MISCELLANEOUS_H_
