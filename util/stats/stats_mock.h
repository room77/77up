// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_STATS_STATS_MOCK_H_
#define _UTIL_STATS_STATS_MOCK_H_

#include "util/stats/stats.h"

#include "base/defs.h"
#include "test/cc/test_main.h"

namespace stats {
namespace test {

class MockStatsInterface : public StatsInterface {
 public:
  MOCK_CONST_METHOD0(size,
      int());
  MOCK_CONST_METHOD0(ReadUptoTime,
      uint64_t());
  MOCK_CONST_METHOD1(ReadUptoTimeForSignal,
      uint64_t(const string& signal));
  MOCK_METHOD1(TimeDecayData,
      bool(uint64_t now));
  MOCK_METHOD2(TimeDecaySignalData,
      bool(const string& signal, uint64_t now));
  MOCK_METHOD3(IncrementKey,
      void(const string& key, double value, bool update_metadata));
  MOCK_METHOD4(IncrementKeyWithTimestamp,
      void(const string& key, uint64_t timestamp, double value, bool update_metadata));
  MOCK_METHOD4(IncrementKeyForSignal,
      void(const string& signal, const string& key, double value, bool update_metadata));
  MOCK_METHOD5(IncrementKeyForSignalWithTimestamp,
      void(const string& signal, const string& key, uint64_t timestamp, double value,
           bool update_metadata));
  MOCK_CONST_METHOD1(GetKey,
      double(const string& key));
  MOCK_CONST_METHOD2(GetKeyForSignal,
      double(const string& signal, const string& key));
  MOCK_METHOD1(RemoveKeysBelowThreshold,
      int(double threshold));
  MOCK_METHOD2(RemoveKeysForSignalBelowThreshold,
      int(const string& signal, double threshold));
  MOCK_METHOD1(FromBinary,
      bool(istream& in));
  MOCK_METHOD1(FromJSON,
      bool(istream& in));
  MOCK_CONST_METHOD1(ToBinary,
      void(ostream& out));
  MOCK_CONST_METHOD2(ToJSON,
      void(ostream& out, const serial::JSONSerializationParams& params));
};

inline void RegisterNewMockStatsInterface(const string& id) {
  StatsInterface::bind(id, []{ return new MockStatsInterface(); });
}

}  // namespace test
}  // namespace stats


#endif  // _UTIL_STATS_STATS_MOCK_H_
