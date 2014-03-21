// Copyright 2014 Room77, Inc.
// Author: Nikola Otasevic

#include "meta/log/offline/filtering/end_point_interface.h"
#include <mutex>
#include "base/defs.h"
#include "util/init/init.h"

// Path where sharded files are to be stored.
FLAG_string(writing_path, "", "writing path");

extern string gFlag_logging_log_root_dir;

namespace logging {
namespace event {

class ShardingEndPoint : public EndPointInterface {
 protected:
  // This function should make a string such as "<reading_path>/20140201/20.log" into
  // "<writing_path>/20140201/<sharded_session_id>"
  virtual string GetEndPoint(const tLogElement& element) const {
    ASSERT(!gFlag_writing_path.empty()) << "Please set the writing path." ;

    string new_path = element.element_add_info.read_filename;
    int index = new_path.rfind(gFlag_logging_log_root_dir);

    // Check that the reading path is not corrupted
    ASSERT(index != std::string::npos) << "Reading path is not as expected";

    // Make a corresponding writing path
    new_path.replace(index, gFlag_logging_log_root_dir.length(), gFlag_writing_path);

    index = new_path.rfind("/");
    string sharded_session_id = "/";
    sharded_session_id += element.session_id.substr(0, 2);
    new_path.replace(index, new_path.length()-index, sharded_session_id);
    return new_path;
  }
};

INIT_ADD_REQUIRED("sharding_end_point", [] {
  // register the end point class to the EndPointInterface factory
  EndPointInterface::bind("ShardingEndPoint", [] { return new ShardingEndPoint(); });

  EndPointInterface::pin(EndPointInterface::make_shared("ShardingEndPoint"));
});

}  // namespace event
}  // namespace logging
