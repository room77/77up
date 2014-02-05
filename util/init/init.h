// Copyright 2013 Room77, Inc.
// Author: Uygar Oztekin

// Initializer / exit registration mechanism for r77.
// See init_test.cc for a basic example.

// There are two main ways to register an intializer:
// - INIT_ADD(groupname, function);
// - INIT_ADD(groupname, priority, function);

// There are two main ways to register a finalizer:
// - EXIT_ADD(groupname, function);
// - EXIT_ADD(groupname, priority, function);

// Groupname must be a C-style string and may only contain lowercase letters,
// numbers and underscore (a-z,0-9,_). No other characters are allowed.
// Priority is an optional integer. Function must be castable to
// std::function<void()> and it typically is a lambda function.

// If your initializer can be run in any order, you should always use the first
// versions (without priority) for best efficiency. This way we can run the most
// amount of initializers in parallel.

// There are infrequent corner cases (e.g. tests or finalizations) that require
// a particular order. In that case you could use the second versions and supply
// a priority. See below

// Here are the rules:
// - Default priority is always 0.
// - Every initializer within the same priority can be run in parallel in an
//   arbitrary order with other initializers of the same priority.
// - Each priority level is run from lower to higher (negative is allowed).
// - Priorities are NOT designed to get around thread safety issues. Every
//   initializer must be thread safe.

// Priority ranges are reserved. Please follow these rules:
// - Do not set a priority if you do not need it (most efficient).
// - [-99, 99]: Can be used if you really must for user code.
//   To be able run maximum number of initializers in parallel, use smallest
//   absolute value possible to guarantee correct operation. If you need to make
//   sure that Y goes before or after X. Do not set priority for X. Set Y's
//   priority to -1 or 1 depending on what you need.
// - [-999, -100] and [100, 999]: Do NOT use without asking. Reserved for
//   special libs.
// - <= -1000 and >=1000 Reserved. Do NOT use.

// In general, if you want to use anything outside of [-99, 99] range, please
// talk to Uygar / Pramod first.

// INIT_ADD_REQUIRED and EXIT_ADD_REQUIRED are similar to INIT_ADD / EXIT_ADD
// except they always run. Groups registered via the required versions cannot be
// suppred by --init_include, --init_exclude, --exit_include, --exit_exclude.
// They should only be used in special and extremely lightweight cases. The only
// use case we have for now is to register server methods. If you would like to
// use required versions for other use cases, please talk to Uygar / Pramod
// first.

#ifndef _PUBLIC_UTIL_INIT_INIT_H_
#define _PUBLIC_UTIL_INIT_INIT_H_

#define INIT_ADD(GROUP, ...) \
  static auto FILE_UNIQUE_TOKEN = ::init::InitAdd(GROUP, FILE_AND_LINE, __VA_ARGS__)

#define EXIT_ADD(GROUP, ...) \
  static auto FILE_UNIQUE_TOKEN = ::init::ExitAdd(GROUP, FILE_AND_LINE, __VA_ARGS__)

#define INIT_ADD_REQUIRED(GROUP, ...) \
  static auto FILE_UNIQUE_TOKEN = ::init::InitAdd("!" GROUP, FILE_AND_LINE, __VA_ARGS__)

#define EXIT_ADD_REQUIRED(GROUP, ...) \
  static auto FILE_UNIQUE_TOKEN = ::init::ExitAdd("!" GROUP, FILE_AND_LINE, __VA_ARGS__)


// Helper macros to generate a unique token per file. We need 3 macros to
// achieve this. Tokens are not guaranteed to be unique across multiple files.
// Hence, INIT_ADD must be used insides unnamed namespaces at all times.
#define FILE_UNIQUE_TOKEN2(a,b,c, d) a ## b ## c ## d
#define FILE_UNIQUE_TOKEN1(a,b,c, d) FILE_UNIQUE_TOKEN2(a, b, c, d)
#define FILE_UNIQUE_TOKEN FILE_UNIQUE_TOKEN1(line_, __LINE__, _counter_, __COUNTER__)


#define FILE_AND_LINE2(l) __FILE__ ":" #l
#define FILE_AND_LINE1(l) FILE_AND_LINE2(l)
#define FILE_AND_LINE FILE_AND_LINE1(__LINE__)

#include <string>
#include <functional>

namespace init {

// Don't use these directly, use the INIT_ADD macro.
bool InitAdd(const std::string& grp, const char* loc, std::function<void()> f);
bool InitAdd(const std::string& grp, const char* loc, int pri, std::function<void()> f);

bool ExitAdd(const std::string& grp, const char* loc, std::function<void()> f);
bool ExitAdd(const std::string& grp, const char* loc, int pri, std::function<void()> f);

}  // namespace init

// Called in the beginning to execute all callbacks registered for execution
// after main begins.
void InitRun();

// Called at the end to execute all callbacks registered for execution
// after main exits.
void ExitRun();

#endif
