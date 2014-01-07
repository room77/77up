// Copyright 2007 OpTrip, Inc.
//
// LOG/ASSERT routines

#ifndef _PUBLIC_BASE_LOGGING_H_
#define _PUBLIC_BASE_LOGGING_H_

#include <chrono>
#include <ctime>
#include <sstream>

#include "base/defs.h"
#include "sysinfo.h"
#include "system.h"
#include "util/time/duration.h"

extern bool gFlag_logtimestamp;
extern bool gFlag_logtrace;
extern int gFlag_loglevel;

enum {
  FATAL = -1000,
  ERROR = 0,
  WARNING = 1,
  INFO = 2,
};

// log macro
//
// LOG macro prints out:
//   current timestamp, source file name, line number and message
#define LOG(level) ((level) > gFlag_loglevel) || \
    (Logger::NewLogger(true, __FILE__, __LINE__, (level) == FATAL))

#define LOG_IF(level, cond) ((level) > gFlag_loglevel && (cond)) || \
    (Logger::NewLogger(true, __FILE__, __LINE__, (level) == FATAL))

#define LOG_EVERY_N(level, n) \
    static uint32_t log_every_n_ ## __COUNTER__ = 0; \
    (level) > gFlag_loglevel || (log_every_n_ ## __COUNTER__++  % (n)) || \
    (Logger::NewLogger(true, __FILE__, __LINE__, (level) == FATAL))

#define ASSERT(expr) (expr) || \
    (Logger::NewLogger(true, __FILE__, __LINE__, true)) << "(" << #expr << ") "

// For now, let's hack those up via LOG and ASSERT. In the future, we may
// implement them properly.
#define VLOG(level) LOG(level)
#define VLOG_IF(level, cond) LOG_IF((level), (cond))

// dev version of assertion: abort in development environment only; keep going
// in production environment
#define ASSERT_DEV(expr) if (!(expr)) \
    Logger::NewLogger(true, __FILE__, __LINE__, \
         !(SysInfo::Instance().InProduction())) << "(" << #expr << ") "

#define ASSERT_DEV_EQ(e1, e2)  ASSERT_DEV((e1) == (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_DEV_NEQ(e1, e2) ASSERT_DEV((e1) != (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_DEV_LT(e1, e2)  ASSERT_DEV((e1) <  (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_DEV_GT(e1, e2)  ASSERT_DEV((e1) >  (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_DEV_LE(e1, e2)  ASSERT_DEV((e1) <= (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_DEV_GE(e1, e2)  ASSERT_DEV((e1) >= (e2)) << "(" << (e1) << " vs. " << (e2) << ") "


// ASSERT_xx family of macros (==, !=, <, >, <=, >=)
#define ASSERT_EQ(e1, e2)  ASSERT((e1) == (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_NEQ(e1, e2) ASSERT((e1) != (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_LT(e1, e2)  ASSERT((e1) <  (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_GT(e1, e2)  ASSERT((e1) >  (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_LE(e1, e2)  ASSERT((e1) <= (e2)) << "(" << (e1) << " vs. " << (e2) << ") "
#define ASSERT_GE(e1, e2)  ASSERT((e1) >= (e2)) << "(" << (e1) << " vs. " << (e2) << ") "

#define ASSERT_NOTNULL(ptr) ASSERT(ptr != nullptr)

class Logger {
 public:
  Logger(bool active, const char *filename, int linenum, bool terminate) :
    active_(active), terminate_(terminate) {
    if (active_) {
      // output timestamp in yyyymmdd|hhmmss format
      if (gFlag_logtimestamp) {

        auto now = std::chrono::system_clock::now();
        std::time_t tnow = std::chrono::system_clock::to_time_t(now);
        char mbstr[100] = {0};
        if (std::strftime(mbstr, 100, "%Y%m%d.%H%M%S", std::localtime(&tnow))) {
          str_ << mbstr;

          auto duration = now.time_since_epoch();
          double usec = std::chrono::duration_cast< ::util::time::second>(duration).count() -
              std::chrono::duration_cast<std::chrono::seconds>(duration).count();
          str_ <<  "." << static_cast<int>(usec * 1000000)  << " ";
        }
      }

      if (terminate_) str_ << "Assertion failed at ";

      if (terminate_ || gFlag_logtrace)
        str_ << filename << ":" << linenum << ": ";
    }
  };

  Logger(const Logger& r) : active_(r.active_), terminate_(r.terminate_), str_(r.str_.str()) {}

  ~Logger() {
    if (terminate_) {
      str_ << endl;
      PrintStackTraceToStream(str_, 1);
    }

    if (active_) cout <<  str_.str() << endl;

    // Force a segfault.
    if (terminate_) ShutdownSystem(2);
  };

  template<typename T>
  Logger& operator<<(T s) {
    if (active_)
      str_ << s;
    return *this;
  }

  // allow Logger to be converted to booleans, so that ASSERT() macro will work
  operator bool() const { return true; }

  static Logger NewLogger(bool active, const char *filename, int linenum, bool terminate) {
    Logger lg(active, filename, linenum, terminate);
    return lg;
  }
 private:
  bool active_;
  bool terminate_;
  ostringstream str_;
};

#endif
