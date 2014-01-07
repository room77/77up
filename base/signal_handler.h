// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_BASE_SIGNAL_HANDLER_H_
#define _PUBLIC_BASE_SIGNAL_HANDLER_H_

#include <cassert>
#include <cstring>
#include <signal.h>

#include <functional>
#include <unordered_map>

#include "base/logging.h"

namespace base {

class SignalHandler {
 public:
  typedef std::function<void ()> SigHandler;

  ~SignalHandler() {}

  static SignalHandler& Instance() {
    static SignalHandler the_one;
    return the_one;
  }

  bool IsHandlerRegistered(int sig) {
    return handlers_.find(sig) != handlers_.end();
  }

  bool Register(int sig,  SigHandler func, int flags = SA_RESTART) {
    ASSERT(handlers_.find(sig) == handlers_.end())
        << "Signal: " << sig << " already registered";

    struct sigaction act = {};
    memset((void*)&act, 0, sizeof(act));
    act.sa_handler = &SignalHandler::Callback;
    act.sa_flags = flags;
    if (sigaction(sig, &act, nullptr) < 0) {
      LOG(ERROR) << "Failed to register signal: " << sig;
      return false;
    }

    handlers_[sig] = func;
    VLOG(3) << "Registered signal: " << sig;
    return true;
  }

 private:
  static void Callback(int sig) { Instance().HandleSignal(sig); }

  void HandleSignal(int sig) {
    VLOG(3) << "Handle: " << sig;
    ASSERT(handlers_.find(sig) != handlers_.end())
        << "Unknown signal: " << sig;
    handlers_[sig]();
  }

 private:
  SignalHandler() {
    stack_t ss;
    static const int kAltStackSize = 1 * 1024 * 1024;  // 1MB
    ss.ss_sp = new char[kAltStackSize];
    ss.ss_flags = 0;
    ss.ss_size = kAltStackSize;
    sigaltstack(&ss, nullptr);
    // ignore broken-pipe errors in case caller closes connection
    // it is CRITICAL to set this globally so processes don't randomly die
    // instead of triggering the signal, an EPIPE error code is returned
    // by the write command
    // overview: http://stackoverflow.com/questions/8829238/how-can-i-trap-a-signal-sigpipe-for-a-socket-that-closes
    signal(SIGPIPE, SIG_IGN);
  }

  std::unordered_map<int, SigHandler> handlers_;
};

}  // namespace base


#endif  // _PUBLIC_BASE_SIGNAL_HANDLER_H_
