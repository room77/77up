#include <signal.h>
#include <sys/stat.h>
#include <time.h>

#include "args/args.h"
#include "util/file/file.h"
#include "logging.h"

void SysInfo::Init() {
  in_production_ = file::Exists("/home/config/prod_machine");
  in_test_ = file::Exists("/localdisk/home/is_r77_test");
  ASSERT(!(in_production_ && in_test_))
      << "Can't be in both production and test environments";

  in_staging_ = file::Exists("/home/config/staging_machine");
  ASSERT(!in_staging_ || in_production_)
      << "Staging machines must be marked as prod";

  LOG(INFO) << "System configured as:" << (in_production_ ? " prod " : " dev ")
    << (in_staging_ ? " staging " : "") << (in_test_ ? " test " : "");
}
