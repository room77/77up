// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: yangc@room77.com (Calvin Yang), pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_BASE_ARGS_ARG_INFO_H_
#define _PUBLIC_BASE_ARGS_ARG_INFO_H_

#include <functional>
#include <future>
#include <mutex>
#include <sstream>
#include <typeindex>

#include "base/defs.h"
#include "base/demangle.h"

namespace args {

// Maintains the information related to a cmd line arg.
class ArgInfo {
 public:
  ArgInfo(const string& name, const string& description) :
    name_(name), description_(description), assigned_(false), edited_(false),
    editable_(true) {};
  virtual ~ArgInfo() {};

  const string& name() const { return name_; }
  const string& description() const { return description_; }
  bool edited() const { return edited_; }
  bool editable() const { return editable_; }

  void HideFromParamEditor() { editable_ = false; }

  string ParseValue(const string& value, bool edit_mode) {
    lock_guard<mutex> l(m_);
    if (assigned_ && !edit_mode)
      return "Duplicate assignment of arg: " + name_;
    assigned_ = true;
    if (edit_mode) {
      // for later restoration
      if (!edited_) old_value_ = GetValueUnlocked();
      edited_ = true;
    }
    return ParseFromStringWithCallbackUnlocked(value);
  }

  string RestoreValue() {
    lock_guard<mutex> l(m_);
    if (edited_) {
      edited_ = false;
      return ParseFromStringWithCallbackUnlocked(old_value_);
    }
    return "";
  }

  // Returns the current value of the arg as string.
  string Value() const {
    lock_guard<mutex> l(m_);
    return GetValueUnlocked();
  }

  void CallBack() const {
    lock_guard<mutex> l(m_);
    CallBackUnlocked();
  }

  // Pure virtual functions that must be implemented by a derived class.

  // type of this argument (int, string, bool, etc.)
  virtual string ArgType() const = 0;

 protected:
  // The functions below can safely assume atomicity.
  // Only one thread can call one of these function at one time.

  // Returns err string on failure and empty on success.
  virtual string ParseFromStringUnlocked(const string& value) = 0;

  virtual string GetValueUnlocked() const = 0;

  virtual void CallBackUnlocked() const = 0;

  // This function updates the value and calls the update callback if the
  // value was parsed successfully.
  virtual string ParseFromStringWithCallbackUnlocked(const string& value) {
    string err = ParseFromStringUnlocked(value);
    if (err.empty()) CallBackUnlocked();
    return err;
  }


  string name_, description_, old_value_;
  bool assigned_, edited_, editable_;
  mutable std::mutex m_;
};

template<typename T>
class ArgInfoWithData : public ArgInfo {
 public:
  typedef std::function<void(const T&)> ArgUpdateCallback;

  ArgInfoWithData(const string& name, T* variable, const T& def_value,
                  const string& description)
      : ArgInfoWithData(name, variable, def_value, ArgUpdateCallback(),
                        description) {}

  ArgInfoWithData(const string& name, T* variable, const T& def_value,
                  ArgUpdateCallback update_callback, const string& description)
      : ArgInfo(name, description), variable_(variable),
        default_value_(def_value), update_callback_(update_callback) {
    *variable = def_value;
  }

  virtual ~ArgInfoWithData() {};

  // type of this argument (int, string, bool, etc.)
  virtual string ArgType() const {
    return base::PrettyNameFromType<T>();
  }

  virtual void CallBackUnlocked() const {
    if (update_callback_) update_callback_(*variable_);
  }

 protected:
  // Returns err string on failure and empty on success.
  virtual string ParseFromStringUnlocked(const string& value);

  virtual string GetValueUnlocked() const;

  T* variable_;
  T default_value_ = T();
  ArgUpdateCallback update_callback_;
};

template<typename T>
string ArgInfoWithData<T>::GetValueUnlocked() const {
  stringstream ss;
  ss << *variable_;
  return ss.str();
}

template<typename T>
string ArgInfoWithData<T>::ParseFromStringUnlocked(const string& value) {
  if (value.empty()) return "Missing value for arg: " + name_;

  stringstream ss(value.c_str());
  ss >> *variable_;
  if (ss.fail())
    return "Failed to parse value (" + value + ") for arg: " + name_;
  return "";
}

// Specialization for string.
template<>
string ArgInfoWithData<string>::GetValueUnlocked() const;

template<>
string ArgInfoWithData<string>::ParseFromStringUnlocked(const string& value);

// Specialization for boolean.
template<>
string ArgInfoWithData<bool>::GetValueUnlocked() const;

template<>
string ArgInfoWithData<bool>::ParseFromStringUnlocked(const string& value);

}  // namespace args


#endif  // _PUBLIC_BASE_ARGS_ARG_INFO_H_
