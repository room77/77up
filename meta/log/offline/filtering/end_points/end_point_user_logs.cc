// Copyright 2014 Room77, Inc.
// Author: Nikola Otasevic

#include "meta/log/offline/filtering/end_point_interface.h"

#include "base/defs.h"
#include "util/init/init.h"

// path for the new group of logs
FLAG_string(writing_path, "", "writing path");

extern string gFlag_logging_log_root_dir;

namespace logging {
namespace event {

// This class provides end point for the user logs
class UserLogsEndPoint : public EndPointInterface {
 protected:
  // This function makes a string such as "<reading_path>/20140201/20.log" into
  // "<writing_path>/20140201/20.log"
  virtual string GetEndPoint(const tLogElement& element) const {

    ASSERT(!gFlag_writing_path.empty()) << "Please set the writing path." ;

    string new_path = element.element_add_info.read_filename;
    int index = new_path.rfind(gFlag_logging_log_root_dir);

    // Check that the reading path is not corrupted
    ASSERT(index != std::string::npos) << "Reading path is not as expected";

    // Make and return a corresponding writing path
    new_path.replace(index, gFlag_logging_log_root_dir.length(), gFlag_writing_path);
    return new_path;
  }
};

INIT_ADD_REQUIRED("user_logs_end_point", [] {
  // register the end point class to the EndPointInterface factory
  EndPointInterface::bind("UserLogsEndPoint", [] { return new UserLogsEndPoint(); });

  EndPointInterface::pin(EndPointInterface::make_shared("UserLogsEndPoint"));
});

}  // namespace event
}  // namespace logging
