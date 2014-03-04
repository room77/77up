// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_TEST_CC_TEST_H_
#define _PUBLIC_TEST_CC_TEST_H_

#include <unordered_set>
#include <chrono>

#include "base/common.h"

#define GTEST_DONT_DEFINE_ASSERT_EQ 1
#define GTEST_DONT_DEFINE_ASSERT_NE 1
#define GTEST_DONT_DEFINE_ASSERT_LE 1
#define GTEST_DONT_DEFINE_ASSERT_LT 1
#define GTEST_DONT_DEFINE_ASSERT_GE 1
#define GTEST_DONT_DEFINE_ASSERT_GT 1

// This is not defined for makedeps, which leads to a warning.
// We enable it here to remove that warning. This will need changes if we
// decide to compile on non-linux platforms.
#ifndef __linux__
#define __linux__
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// The test dir that can be used to create temporary files.
// The directory is deleted at the end.
extern string gFlag_test_dir;

template<class T>
void EXPECT_IN(const T& el, const unordered_set<T>& possibilities) {
  EXPECT_NE(possibilities.find(el), possibilities.end()); \
}

namespace test {

template <typename T = std::chrono::milliseconds>
struct BenchMark {
  BenchMark(const string& str) : str_(str),
      begin_(std::chrono::high_resolution_clock::now()) {}

  ~BenchMark() {
    auto elapsed = std::chrono::duration_cast<T>(
        std::chrono::high_resolution_clock::now() - begin_);
    LOG(INFO) << " Time: " << str_ << ":" << elapsed.count();
  }

  const string str_;
  std::chrono::time_point<std::chrono::high_resolution_clock> begin_;
};

}  // namespace test

#endif  // _PUBLIC_TEST_CC_TEST_H_
