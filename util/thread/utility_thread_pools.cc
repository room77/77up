// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/thread/utility_thread_pools.h"
#include "base/args/args.h"

FLAG_int(non_time_critical_tasks_pool_size, 32,
         "Number of threads to use in the non critical task thread pool.");
