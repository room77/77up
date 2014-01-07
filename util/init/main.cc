// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <cstdlib>

#include "base/args/args.h"
#include "base/logging.h"
#include "init.h"
#include "base/signal_handler.h"
#include "base/system.h"

FLAG_bool(r77_run_main, true, "If false, don't run main. Init and exit only.");

namespace r77 {

// Register handler for SIGSEV.
bool reg_sigsev = base::SignalHandler::Instance().Register(
    SIGSEGV, []() { PrintStackTrace(3); }, SA_RESETHAND | SA_ONSTACK);

bool reg_sigabrt = base::SignalHandler::Instance().Register(
    SIGABRT, []() { PrintStackTrace(3); }, SA_RESETHAND | SA_ONSTACK);

void R77Shutdown();

void R77Init(int argc, char** argv) {
  args::ArgsInit(argc, argv);

  // initialize random seed.
  // TODO(pramodg): Deprecate all rand() calls in favor of c++11 <random>.
  srand(time(nullptr));

  // This should be the last thing we do.
  InitRun();
}

void R77Shutdown() {
  static bool ran_before = false;
  if (ran_before) return;
  ran_before = true;
  LOG(INFO) << "Shutdown";
  // This should be the first thing we do at exit.
  ExitRun();
}

}  // namespace r77

// init_main() and main() are marked weak which means that user can define
// either main() or init_main() for backwards compatibility or when we want to
// use main() instead (e.g. for code we opensource or tests using gtest).
__attribute__ ((weak))
int init_main() {
  ASSERT(false)
    << "User must define int init_main() or int main(int argc, char** argv)";
  return 1;
}

__attribute__ ((weak))
int main(int argc, char** argv) {
  r77::R77Init(argc, argv);
  int status = gFlag_r77_run_main ? init_main() : 0;
  r77::R77Shutdown();
  return status;
}

INIT_ADD_REQUIRED("register_exit", 0, []{
     atexit(r77::R77Shutdown); at_quick_exit(r77::R77Shutdown);
});

