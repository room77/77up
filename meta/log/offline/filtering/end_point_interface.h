// Copyright 2014 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_OFFLINE_FILTERING_END_POINT_INTERFACE_H_
#define _PUBLIC_META_LOG_OFFLINE_FILTERING_END_POINT_INTERFACE_H_

#include "base/defs.h"
#include "meta/log/common/log_datatypes.h"
#include "util/factory/factory.h"

namespace logging {
namespace event {

// interface that enables factory registration for end points
class EndPointInterface : public Factory<EndPointInterface> {
 public:
  virtual ~EndPointInterface() {}

  virtual string GetEndPoint(const tLogElement& element) const = 0;
};

}  // namespace event
}  // namespace logging

#endif  // _PUBLIC_META_LOG_OFFLINE_FILTERING_END_POINT_INTERFACE_H_
