// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: yangc@room77.com (Calvin Yang), pramodg@room77.com (Pramod Gupta)

// Class to handle command-line flags
//
// Sample usage:
//
// Definition:
//   FLAG_int(var1, 0, "a test variable");
//   FLAG_string(var2, "hello", "another test variable");
//   FLAG_bool(var3, false, "another test variable");
//
// Activation at beginning of main():
//   Init(argc, argv);
//
// Usage:
//   command-line argument specification:
//     ./foo_unittest --var1=1 --var2=hi --var3=true
//
//   gFLAG_var1, gFLAG_var2, gFLAG_var3 can be used to refer to variables
//   specified in the command line
//

#ifndef _PUBLIC_BASE_ARGS_ARGS_H_
#define _PUBLIC_BASE_ARGS_ARGS_H_

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include "arg_info.h"
#include "util/templates/singleton.h"

namespace args {

class CommandLineArgs : public ::util::tl::Singleton<CommandLineArgs> {
  typedef unordered_map<string, shared_ptr<ArgInfo>> ArgNameInfoMap;

public:
  CommandLineArgs() {};

  // Initialize argument list from command-line variables.
  void Initialize(int argc, char **argv);

  // Register a new command line argument.
  template<typename T>
  bool Register(const string& name, T *arg, const T& default_value,
                const string& description) {
    AddArgument(new ArgInfoWithData<T>(name, arg, default_value, description));
    return true;
  }

  // Register a new command line argument with update callback.
  template<typename T>
  bool Register(const string& name, T *arg, const T& default_value,
                std::function<void(const T&)> update_callback,
                const string& description) {
    AddArgument(new ArgInfoWithData<T>(name, arg, default_value,
                                       update_callback, description));
    return true;
  }

  bool HideFromParamEditor(const string& var_name);

  const vector<const ArgInfo*>& GetArgsSorted() const { return sorted_args_; }

  // Print out argument description.
  void PrintUsage() const;

  // Add a new record (of any type derived from ArgInfo) to the arg info map.
  void AddArgument(ArgInfo* new_arg);

  // retrieve a record from variable map
  inline ArgInfo* GetArgument(const string& name) {
    ArgNameInfoMap::iterator i = argname_info_map_.find(name);
    if (i != argname_info_map_.end()) return i->second.get();
    return nullptr;
  }

  const ArgNameInfoMap& arg_name_info_map() const { return argname_info_map_; }

  // assign a command-line argument (given (name, value) pair)
  string AssignArgument(const string& name, const string& value, bool edit_mode);

  string RestoreArgumentValue(const string& name);

  // get current value (displayed as string)
  string GetArgumentValue(const string& name);

  ArgNameInfoMap argname_info_map_;
  vector<const ArgInfo*> sorted_args_;  // sorted array of arguments
};

// Initializer function to setup commnad line arguments.
// This must be called before we init logging to setup the flags correctly.
void ArgsInit(int argc, char **argv);

}  // namespace args

// Macros for registering flags.
#define FLAG_type(type, var, ...) \
  type gFlag_ ## var = type(); \
  bool __reg_ ## var = ::args::CommandLineArgs::Instance().Register<type>( \
      #var, &gFlag_ ## var, __VA_ARGS__)

#define FLAG_bool(var, ...)  FLAG_type(bool, var, __VA_ARGS__)
#define FLAG_int(var, ...)  FLAG_type(int, var, __VA_ARGS__)
#define FLAG_uint32_t(var, ...)  FLAG_type(uint32_t, var, __VA_ARGS__)
#define FLAG_uint64_t(var, ...)  FLAG_type(uint64_t, var, __VA_ARGS__)
#define FLAG_float(var, ...)  FLAG_type(float, var, __VA_ARGS__)
#define FLAG_double(var, ...)  FLAG_type(double, var, __VA_ARGS__)

#define FLAG_string(var, ...)  FLAG_type(::std::string, var, __VA_ARGS__)

#define HIDE_FROM_PARAM_EDITOR(var)  \
  bool __hide_ ## var = \
      ::args::CommandLineArgs::Instance().HideFromParamEditor(#var)


#endif  // _PUBLIC_BASE_ARGS_ARGS_H_
