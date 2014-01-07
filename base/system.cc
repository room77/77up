// Copyright 2007-2013 Optrip, Room77

// Copied from base/small_util

#include "system.h"

#include <cstdlib>
#include <fcntl.h>
#include <execinfo.h>
#include <unistd.h>

using namespace std;

const string& GetExecutableName() {
  struct InitExeName {
    string operator()() {
      static const int kMaxSize = 1024 * 2;
      char path[kMaxSize] = {0};
      if (readlink("/proc/self/exe", path, kMaxSize) != -1) return path;
      return "";
    }
  };

  static string executable_name = InitExeName()();
  return executable_name;
}

int RunNonBlockingPipedCommand(const string& cmd, string* stdout,
                               int timeout_sec) {
  int res = -1;
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) return res;
  int fd = fileno(fp);
  // Make the pipe non-blocking.
  fcntl(fd, F_SETFL, O_NONBLOCK);
  const int max_buffer = 2048;
  char buffer[max_buffer];
  time_t start_time = time(NULL);
  while ((time(NULL) - start_time) < timeout_sec) {
    buffer[0] = '0';
    ssize_t r = read(fd, buffer, max_buffer);
    // Note that errno is thread local.
    if (r == -1 && errno == EAGAIN) continue;
    else if (r > 0) stdout->append(buffer);
    else break;
  }

  int status = pclose(fp);
  if (WIFSIGNALED(status) != 0) {
    cout << "Cmd ["  << cmd << "] terminated by signal: "  << WTERMSIG(status) << endl;
  } else if (WIFEXITED(status)) {
    // cout << "Cmd ["  << cmd << "] status: "  << WEXITSTATUS(status) << endl;
    res = WEXITSTATUS(status);
  } else {
    cout << "Cmd ["  << cmd << "] terminated abormally with status: "  << status << endl;
    res = status;
  }
  return res;
}

string GetStackTrace(int ignore_n_frames) {
  const int max_stacks_to_show = 100;
  void* stack_trace[max_stacks_to_show];
  int nptrs = backtrace(stack_trace, max_stacks_to_show);
  stringstream trace;
  for (int i = ignore_n_frames; i < nptrs; ++i)
    trace << " " << stack_trace[i];
  return trace.str();
}

void ShutdownSystem(int status) {
  quick_exit(status);  // shut down server
}
