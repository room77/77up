// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Test utils for serialization tests.

#ifndef _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_TEST_UTIL_H_
#define _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_TEST_UTIL_H_

#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "base/common.h"
#include "test/cc/unit_test.h"
#include "util/serial/types/varint.h"

namespace serial {
namespace test {

struct TestDataWithZeroDefaults {
  int test_arr[10] = {0};
  bool test_bool = false;
  char test_char = 0;
  int test_int = 0;
  map<int, string> test_map;
  set<int> test_set;
  string test_str;

  bool operator == (const TestDataWithZeroDefaults& rhs) const {
    return (memcmp(test_arr, rhs.test_arr, 10) == 0 &&
        test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_int == rhs.test_int && test_map == rhs.test_map &&
        test_set == rhs.test_set && test_str == rhs.test_str);
  }
};

// Basic data for different tests.
struct TestDataWithCustomDefaults {
  int test_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  bool test_bool = true;
  char test_char = 'A';
  int test_int = 1;
  map<int, string> test_map = {{1, "S1"}, {2, "S2"}, {3, "S3"}, {4, "S4"},
                               {5, "S5"}};
  set<int> test_set = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  string test_str = "some_string";

  bool operator == (const TestDataWithCustomDefaults& rhs) const {
    return (memcmp(test_arr, rhs.test_arr, 10) == 0 &&
        test_bool == rhs.test_bool && test_char == rhs.test_char &&
        test_int == rhs.test_int && test_map == rhs.test_map &&
        test_set == rhs.test_set && test_str == rhs.test_str);
  }
};

// Utility class to serialize both the classes above.
template<typename T = TestDataWithZeroDefaults>
struct TestDataWithSerialization : public T {
  void ToBinary(ostream& out) const {
    // We only serialize the simple parts. Make sure the string does not have
    // extra spaces.
    out << this->test_int << " " << this->test_char << " " << this->test_str
       << " " << this->test_bool;
  }

  bool FromBinary(istream& in) {
    // We only serialize the simple parts.
    in >> this->test_int >> this->test_char >> this->test_str >> this->test_bool;
    return !in.fail();
  }

  void ToJSON(ostream& ss) const {
    return ToBinary(ss);
  }

  bool FromJSON(istream& ss) {
    return FromBinary(ss);
  }
};

// Utility class to serialize with callback.
template<typename T = TestDataWithZeroDefaults>
struct TestDataCallback : public TestDataWithSerialization<T> {
  bool callback_ = false;

  bool DeserializationCallback() {
    callback_ = true;
    return true;
  }
};

// Utility class to test using serialization defined in a super class.
template<typename T = TestDataWithZeroDefaults>
struct TestDataWithSerializationDerived : public TestDataWithSerialization<T> {};


// Logging for TestDataWithSerialization.
template<typename T = TestDataWithZeroDefaults>
std::ostream& operator<<(std::ostream& os, const TestDataWithSerialization<T>& v) {
  os << "Arr: [";
  for (int a : v.test_arr) os << a << ", ";
  os << "], Bool: " << v.test_bool;
  os << ", Char: " << v.test_char;
  os << ", Int: " << v.test_int;
  os << ", Map: { ";
  for (const auto& p : v.test_map) os << "<" << p.first << ", " << p.second << ">, ";

  os << "}, Set: [ ";
  for (int a : v.test_set) os << a << ", ";

  os << "], String: [" << v.test_str << "]";

  return os;
}

typedef testing::Types<int, char, bool, long, unsigned int, size_t,
    unsigned long> AllIntegerTypes;

typedef testing::Types<float, double> AllFloatingTypes;

typedef testing::Types<fixedint<int>, fixedint<long>, fixedint<size_t>> AllFixedTypes;

typedef testing::Types<varint<int>, varint<long>, varint<size_t>> AllVarTypes;

typedef testing::Types<int, char, bool, double, float, long,
    unsigned int, size_t, unsigned long> AllArithmeticTypes;

typedef testing::Types<vector<int>,
    map<string, string>, unordered_map<string, string>,
    set<string>, unordered_set<string>> AllContainerTypes;

typedef testing::Types<shared_ptr<int>, unique_ptr<int>> AllPointerTypes;

typedef testing::Types<int, char, bool, double, float, long,
    unsigned int, size_t, unsigned long,
    pair<int, string>,
    vector<int>,
    map<string, string>, unordered_map<string, string>,
    set<string>, unordered_set<string>,
    shared_ptr<int>, unique_ptr<string>,
    TestDataWithZeroDefaults, TestDataWithCustomDefaults> AllTypes;

inline string print_hex(const string& str) {
  stringstream ss;
  for (const char  c : str) {
    unsigned short a = c;
    ss << "\\x" << hex << setfill('0') << setw(2) << uppercase << a;
  }
  return ss.str();
}

}  // namespace test
}  // namespace serial


#endif  // _PUBLIC_UTIL_SERIAL_TYPE_HANDLERS_TEST_UTIL_H_
