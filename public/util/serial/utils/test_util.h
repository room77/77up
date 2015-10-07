// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test util for serializers.

#ifndef _PUBLIC_UTIL_SERIAL_UTILS_TEST_UTIL_H_
#define _PUBLIC_UTIL_SERIAL_UTILS_TEST_UTIL_H_

#include <initializer_list>
#include <memory>
#include <vector>

#include "base/common.h"
#include "util/serial/serializer_macros.h"

namespace serial {
namespace test {

template<typename T>
struct TestData {
  TestData(const T& d = T()) : data(d) {}  // NOLINT

  template<typename T2>
  bool operator ==(const TestData<T2>& rhs) const {
    // LOG(INFO) << data << ", " << rhs.data;
    return data == rhs.data;
  }

  bool operator ==(const T& rhs) const {
    return data == rhs;
  }

  T data;

  SERIALIZE(SERIALIZE_ALWAYS / data*1);
};

template<typename T, size_t N>
struct TestData<T[N]> {
  TestData(const std::initializer_list<T>& l = {}) {  // NOLINT
    ASSERT_LE(l.size(), N);
    int count = 0;
    for (const T& item : l) data[count++] = item;
    for (; count < N; ++count) data[count] = T();
  }

  bool operator ==(const TestData<T[N]>& rhs) const {
    for (size_t i = 0; i < N; ++i)
      if (!(data[i] == rhs.data[i])) return false;
    return true;
  }

  SERIALIZE(SERIALIZE_ALWAYS / data*1);

  T data[N];
};

template<typename T>
struct TestData<unique_ptr<T> > {
  TestData(const T& d = T()) : data(new T(d)) {}

  bool operator ==(const TestData<unique_ptr<T>>& rhs) const {
    return *data == *rhs.data;
  }

  SERIALIZE(SERIALIZE_ALWAYS / data*1);

  unique_ptr<T> data;
};

template<typename T>
struct TestNested {
  template<typename T2>
  bool operator == (const TestNested<T2>& rhs) const {
    if (test_basic.size() != rhs.test_basic.size()) return false;

    for (int i = 0; i < test_basic.size(); ++i)
      if (!(test_basic[i] == rhs.test_basic[i])) return false;

    return true;
  }
  SERIALIZE(SERIALIZE_ALWAYS / test_basic*1);

  vector<T> test_basic;
};

struct TestDataBasic {
  TestDataBasic(bool test_bool = false, char test_char = 0, int test_int = 0,
                const string& test_str = "")
      : test_bool(test_bool), test_char(test_char), test_int(test_int),
        test_str(test_str) {}

  bool test_bool = false;
  char test_char = 0;
  int test_int = 0;
  string test_str;

  bool operator == (const TestDataBasic& rhs) const {
    return (test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_int == rhs.test_int && test_str == rhs.test_str);
  }

  SERIALIZE(SERIALIZE_ALWAYS / test_bool*1 / test_char*2 / test_int*3 /
            test_str*4);
};

struct TestDataFieldRemoved {
  bool test_bool = false;
  char test_char = 0;
  string test_str;

  bool operator == (const TestDataFieldRemoved& rhs) const {
    return (test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_str == rhs.test_str);
  }

  bool operator == (const TestDataBasic& rhs) const {
    return (test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_str == rhs.test_str);
  }

  SERIALIZE(SERIALIZE_ALWAYS / test_bool*1 / test_char*2 / test_str*4);
};

struct TestDataFieldAdded {
  bool test_bool = false;
  char test_char = 0;
  int test_int = 0;
  string test_str;
  int test_added = -1;

  bool operator == (const TestDataFieldAdded& rhs) const {
    return (test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_int == rhs.test_int && test_str == rhs.test_str &&
        test_added == rhs.test_added);
  }

  bool operator == (const TestDataBasic& rhs) const {
    return (test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_int == rhs.test_int && test_str == rhs.test_str);
  }

  SERIALIZE(SERIALIZE_ALWAYS / test_bool*1 / test_added*5/ test_char*2 /
            test_int*3 / test_str*4);
};

struct TestDataFieldAddedRemoved {
  bool test_bool = false;
  int test_int = 0;
  string test_str;
  int test_added = -1;

  bool operator == (const TestDataFieldAdded& rhs) const {
    return (test_bool == rhs.test_bool &&
        test_int == rhs.test_int && test_str == rhs.test_str &&
        test_added == rhs.test_added);
  }

  bool operator == (const TestDataBasic& rhs) const {
    return (test_bool == rhs.test_bool &&
        test_int == rhs.test_int && test_str == rhs.test_str);
  }

  SERIALIZE(test_bool*1 / test_added*5/ test_int*3 / test_str*4);
};

struct TestDataWithCallback : public TestDataBasic {
  bool return_value_on_callback = true;
  bool callback = false;

  bool DeserializationCallback() {
    callback = true;
    return return_value_on_callback;
  }

  template<typename T>
  bool operator == (const T& rhs) const {
    return (test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_int == rhs.test_int && test_str == rhs.test_str);
  }

  SERIALIZE(SERIALIZE_ALWAYS / test_bool*1 / test_char*2 / test_int*3 /
            test_str*4);
};

struct TestDataPrettyName : public TestDataBasic {
  SERIALIZE(SERIALIZE_ALWAYS / SERIALIZE_TYPEINFO / test_bool*1);
};

struct TestDataRequiredFields : public TestDataBasic {
  using TestDataBasic::TestDataBasic;

  SERIALIZE(SERIALIZE_ALWAYS / test_bool*1 / SERIALIZE_REQUIRED / test_char*2 / test_int*3 /
            SERIALIZE_OPTIONAL / test_str*4);
};

struct TestDataVirtualBase {
  virtual ~TestDataVirtualBase() {}

  int test_int = 0;
  SERIALIZE_VIRTUAL(SERIALIZE_ALWAYS / test_int*1);
};

struct TestDataVirtualDerived : public TestDataVirtualBase {
  virtual ~TestDataVirtualDerived() {}

  float test_float = 0.0;
  SERIALIZE_VIRTUAL(SERIALIZE_ALWAYS / test_int*1 / test_float*2);
};

}  // namespace test
}  // namespace serial

#endif  // _PUBLIC_UTIL_SERIAL_UTILS_TEST_UTIL_H_
