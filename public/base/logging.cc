// Copyright 2007 OpTrip, Inc.

#include "logging.h"
#include "args/args.h"

FLAG_int(loglevel, 2, "level of debug output");
FLAG_bool(logtimestamp, true, "include timestamp in log output");
FLAG_bool(logtrace, true, "include file name and line number in log output");
