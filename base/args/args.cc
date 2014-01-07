// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: yangc@room77.com (Calvin Yang), pramodg@room77.com (Pramod Gupta)

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include "args.h"
#include "base/logging.h"

namespace args {

void ArgsInit(int argc, char** argv) {
  CommandLineArgs::Instance().Initialize(argc, argv);
}

// put knobs before non-knobs, and sort by name within each group
int order_args_by_name(const ArgInfo* a1, const ArgInfo* a2) {
  // Make sure "all logging variables come to the top."
  size_t a1_pos = a1->name().find("log");
  size_t a2_pos = a2->name().find("log");

  if (a1_pos < a2_pos) return true;
  else if (a2_pos < a1_pos) return false;

  return a1->name() < a2->name();
}

// hide from parameter editor
bool CommandLineArgs::HideFromParamEditor(const string& var_name) {
  ArgInfo* a = GetArgument(var_name);
  ASSERT(a != nullptr) << "Unrecognized variable: " << var_name;

  a->HideFromParamEditor();
  return true;
}

// add a new record (of any type derived from ArgInfo) to the variable map
void CommandLineArgs::AddArgument(ArgInfo* new_arg) {
  // check if this name already exists
  ASSERT(argname_info_map_.find(new_arg->name()) == argname_info_map_.end())
      << "Duplicate argument name found: " << new_arg->name();

  argname_info_map_[new_arg->name()] = shared_ptr<ArgInfo>(new_arg);
}

// assign a command-line argument (given (name, value) pair)
string CommandLineArgs::AssignArgument(const string& name, const string& value,
                                       bool edit_mode) {
  LOG(INFO) << "Assigning " << name << "=" << value;
  ArgInfo* info = GetArgument(name);
  if (info != nullptr) return info->ParseValue(value, edit_mode);

  return "Undefined command-line argument: " + name + " = " + value;
}

// restore initial value
string CommandLineArgs::RestoreArgumentValue(const string& name) {
  ArgInfo* info = GetArgument(name);
  if (info != nullptr) return info->RestoreValue();

  return "Undefined command-line argument: " + name;
}

// get current value (displayed as string)
string CommandLineArgs::GetArgumentValue(const string& name) {
  ArgInfo* info = GetArgument(name);
  if (info != nullptr) return info->Value();

  return "Undefined command-line argument: " + name;
}

// print out argument description
void CommandLineArgs::PrintUsage() const {
  cerr << "Command-line arguments:";
  for (const auto& pair : argname_info_map_) {
    const shared_ptr<ArgInfo>& info = pair.second;
    cerr << "  --" << info->name() << " (" << info->ArgType() << "): Default = "
         << info->Value() << ": " << info->description() << endl;
  }
}

// initialize argument list from command-line variables
void CommandLineArgs::Initialize(int argc, char* *argv) {
  for (int i = 1; i < argc; i++) {
    const char* s = argv[i];

    // skip the first two "-"s, if they exist
    if (*s == '-') s++;
    if (*s == '-') s++;

    if (!strcmp(s, "help")) {
      PrintUsage();
      exit(1);
    }

    const char* equal_sign = strchr(s, '=');
    string name(s), value;

    if (equal_sign != nullptr) {
      name = name.substr(0, equal_sign - s);
      value = string(equal_sign + 1);
    }

    string error_msg = AssignArgument(name, value, false);
    ASSERT(error_msg.empty()) << "Failed to assign value (" << name
        << ") to argument (" << value << "). Err : " << error_msg;
  }

  // sort all arguments based on name
  sorted_args_.clear();
  sorted_args_.reserve(argname_info_map_.size());
  for (const auto& pair : argname_info_map_) {
    const shared_ptr<ArgInfo>& info = pair.second;
    sorted_args_.push_back(info.get());

    // We do this explicitly to ensure the callback is called for all vars once
    // after all the flags have been registered and are ready for use.
    info->CallBack();
  }

  // Order all the arguments by name.
  sort(sorted_args_.begin(), sorted_args_.end(), order_args_by_name);
}

}  // namespace args
