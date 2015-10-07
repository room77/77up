// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "base/defs.h"

#include "meta/log/offline/stats/incremental_log_stats_updater.h"

int init_main() {
  logging::offline::IncrementalLogStatsUpdater updater;
  return updater.Update();
}
