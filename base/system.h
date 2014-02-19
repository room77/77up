// Copyright 2007-2013 Optrip, Room77

#ifndef _PUBLIC_BASE_SYSTEM_H_
#define _PUBLIC_BASE_SYSTEM_H_

#include <iostream>
#include <sstream>
#include <string>

const std::string& GetExecutableName();

// Returns the status of the command and fills the stdout after running the
// command. Note: If you also want stderr append '2>&1' at the end.
// Returns '-1' if popen fails.
inline int RunPipedCommand(const std::string& cmd, std::string* stdout) {

  int res = -1;
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) return res;

  const int max_buffer = 256;


  char buffer[max_buffer];
  while (!feof(fp))
    if (fgets(buffer, max_buffer, fp) != NULL) stdout->append(buffer);

  int status = pclose(fp);
  if (WIFSIGNALED(status) != 0) {
    std::cerr << "Cmd ["  << cmd << "] terminated by signal: "  << WTERMSIG(status) << std::endl;
  } else if (WIFEXITED(status)) {
    // cerr << "Cmd ["  << cmd << "] status: "  << WEXITSTATUS(status) << endl;
    res = WEXITSTATUS(status);
  } else {
    std::cerr << "Cmd ["  << cmd << "] terminated abormally with status: "  << status << std::endl;
    res = status;
  }
  return res;
}

// Returns the status of the command and fills the stdout after running the
// command. Note: If you also want stderr append '2>&1' at the end.
// Returns '-1' if popen fails.
int RunNonBlockingPipedCommand(const std::string& cmd, std::string* stdout, int timeout_sec = 10);

// Get stack trace addresses.
std::string GetStackTrace(int ignore_n_frames=0);

inline void PrintStackTraceToStream(const std::string& trace, std::ostream& str) {
  // construct "addr2line" command to print stack trace
  std::stringstream addr2line_cmd;
  const std::string& exe = GetExecutableName();
  addr2line_cmd << "addr2line -i -e " << exe << trace;
  std::string cmd = addr2line_cmd.str() + " | sed 's/^/** /1'";

  std::string out;
  if (!exe.empty()) RunPipedCommand(cmd.c_str(), &out);


  str << "********** Stack Trace **********\n**" << std::endl;
  if (out.empty())
    str << "Could not compute stack trace. \nRun the command below manually." << std::endl;
  else
    str << out << std::endl;
  str << "**\nStack trace obtained by:\n" << addr2line_cmd.str() << std::endl;
  str << "**********************************" << std::endl;
}

inline void PrintStackTraceToStream(std::ostream& str, int ignore_n_frames=0) {
  const std::string trace = GetStackTrace(ignore_n_frames + 1);
  PrintStackTraceToStream(trace, str);
}

// print stack trace to stdout
inline void PrintStackTrace(int ignore_n_frames=0) {
  PrintStackTraceToStream(std::cerr, ignore_n_frames);
}

// Do a system shutdown.
void ShutdownSystem(int status = 0);

// Force a SegFault.
inline void ForceSegFault() { *((volatile char*)nullptr) = 0; }

#endif  // _PUBLIC_BASE_SYSTEM_H_
