// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <signal.h>
#include <chrono>

#include "util/thread/thread_stack.h"

#include "base/signal_handler.h"
#include "util/thread/util.h"
#include "util/time/simple_timer.h"

namespace util {
namespace threading {

string ThreadStack::GetTraceForThreadIds(const vector<int>& thread_ids) {
  std::lock_guard<mutex> l(mutex_);
  counter_.Reset();
  stacks_.clear();

  counter_.ChangeBy(thread_ids.size());
  // Send signal to all threads.
  for (int tid : thread_ids) {
    counter_.ChangeBy(kill(tid, SIGUSR1));  // On error kill returns -1.
    this_thread::sleep_for(chrono::microseconds(100));
  }

  // Wait for at most 5 secs.
  counter_.WaitWithTimeout(5 * 1000);
  VLOG(3) << "Did not finish = " << counter_.count();

  // Swap the contents of the stack.
  map<string, set<std::thread::id>> stacks;
  { std::lock_guard<std::mutex> sl(stacks_mutex_);
    stacks_.swap(stacks);
  }

  stringstream strm;
  strm << "Total Threads: " << thread_ids.size() << endl;
  strm << "No Stack trace for : " << counter_.count() << endl;
  for (const auto& p : stacks) {
    strm << "\n########## Threads Ids: " << p.second.size() << " ##########" << endl;
    for (const std::thread::id& tid : p.second) strm << tid << ", ";
    PrintStackTraceToStream(p.first, strm);
    strm << "##################################\n" << endl;
  }
  return strm.str();
}

string ThreadStack::GetTraceForAllThreads() {
  return GetTraceForThreadIds(util::GetAllThreadIds<vector<int> >());
}

void ThreadStack::AddTraceForCurrentThread() {
  // LOG(INFO) << std::this_thread::get_id();
  ::util::time::SimpleTimer timer;
  timer.Start();
  string trace = GetStackTrace(6);
  int count = 0;
  { lock_guard<mutex> l(stacks_mutex_);
    stacks_[trace].insert(std::this_thread::get_id());
    timer.Stop();
    counter_.Decrement();
    count = counter_.count();
  }
  VLOG(3) << "T:" << timer.GetDurationSec() << ", p:" << count;
}

// Register handler for SIGUSR1.
bool reg_sigsev = base::SignalHandler::Instance().Register(
    SIGUSR1, []{ ThreadStack::Instance().AddTraceForCurrentThread(); });

}  // namespace threading
}  // namespace util
