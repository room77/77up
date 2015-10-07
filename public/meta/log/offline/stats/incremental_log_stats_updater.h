// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_STATS_INCREMENTAL_LOG_STATS_UPDATER_H_
#define _PUBLIC_META_LOG_OFFLINE_STATS_INCREMENTAL_LOG_STATS_UPDATER_H_

#include <memory>

#include "base/defs.h"
#include "util/stats/stats_file.h"
#include "util/time/localtime.h"

namespace logging {
namespace offline {

class IncrementalLogStatsUpdater {
 public:
  // Returns 0 on success.
  int Update();

 protected:
  // Initialized the stats updater.
  int Initialize();
  int ParseFlags();

  // Reads the logs.
  int ReadLogs();

  // Postprocesses the stats.
  int PostProcess();

  // Writes the logs.
  int WriteFiles();

  vector<shared_ptr<stats::StatsFile>> stats_files;
  LocalDate begin_date, end_date;
  vector<string> input_files, output_files;
};

}  // namespace offline
}  // namespace logging


#endif  // _PUBLIC_META_LOG_OFFLINE_STATS_INCREMENTAL_LOG_STATS_UPDATER_H_
