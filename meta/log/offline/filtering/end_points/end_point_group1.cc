// Copyright 2014 Room77, Inc.
// Author: Nikola Otasevic

#include "meta/log/offline/filtering/end_point_interface.h"

#include "base/defs.h"
#include "util/init/init.h"

namespace logging {
namespace event {

class Group1EndPoint : public EndPointInterface {
 protected:
  virtual string GetEndPoint(const tLogElement& element) const {
    // TODO(otasevic, edelman): in addition to the email I sent you about rdbuf, you will likely
    // notice performance gains by writing to localdisk. e.g. on titan we can have
    // /home/share/data/logs/filters be a symlink to /localdisk for better performance at the
    // cost of less persistence
    // TODO(otasevic): return end point path based on the element_info.read_filename
    return "/home/otasevic/Desktop/filtered_logs.txt";  // TODO(otasevic): remove this after testing
  }
};

INIT_ADD_REQUIRED("group1_end_point", [] {
  // register the end point class to the EndPointInterface factory
  EndPointInterface::bind("Group1EndPoint", [] { return new Group1EndPoint(); });

  EndPointInterface::pin(EndPointInterface::make_shared("Group1EndPoint"));
});

}  // namespace event
}  // namespace logging
