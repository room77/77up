// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/time/decay/time_decay.h"

#include <chrono>

#include "test/cc/test_main.h"

namespace decay {
namespace test {

namespace {

// Integer values are easier for testing.
typedef std::chrono::duration<int, std::ratio<86400>> integer_day;

template<typename T = integer_day>
typename T::rep GetTimeDurationRep(int delta) {
  T res = chrono::duration_cast<T>(chrono::high_resolution_clock::now().time_since_epoch()) -
      T(delta);
  return res.count();
}

template<typename T = integer_day>
T GetTimeDuration(int delta) {
  return chrono::duration_cast<T>(chrono::high_resolution_clock::now().time_since_epoch()) -
      T(delta);
}

}  // namespace


TEST(TimeDecay, ExponentialDecayTestNinetyDays) {
  TimeDecay<integer_day, ExponentialDecay> decay(ExponentialDecay(90));

  EXPECT_DOUBLE_EQ(90, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(62.383246250395075, decay.HalfLife());

  EXPECT_DOUBLE_EQ(0.84648172489061413, decay.Decay(GetTimeDurationRep(15)));
  EXPECT_DOUBLE_EQ(0.71653131057378927, decay.Decay(GetTimeDurationRep(30)));
  EXPECT_DOUBLE_EQ(0.51341711903259202, decay.Decay(GetTimeDurationRep(60)));
  EXPECT_DOUBLE_EQ(0.36787944117144233, decay.Decay(GetTimeDurationRep(90)));
  EXPECT_DOUBLE_EQ(0.18887560283756183, decay.Decay(GetTimeDurationRep(150)));
  EXPECT_DOUBLE_EQ(0.1353352832366127, decay.Decay(GetTimeDurationRep(180)));
  EXPECT_DOUBLE_EQ(0.018315638888734186, decay.Decay(GetTimeDurationRep(360)));
}

TEST(TimeDecay, ExponentialDecayTestOneEightyDays) {
  TimeDecay<integer_day, ExponentialDecay> decay(ExponentialDecay(180));
  EXPECT_DOUBLE_EQ(180, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(124.76649250079015, decay.HalfLife());
  EXPECT_DOUBLE_EQ(0.92004441462932329, decay.Decay(GetTimeDurationRep(15)));
  EXPECT_DOUBLE_EQ(0.84648172489061413, decay.Decay(GetTimeDurationRep(30)));
  EXPECT_DOUBLE_EQ(0.71653131057378927, decay.Decay(GetTimeDurationRep(60)));
  EXPECT_DOUBLE_EQ(0.60653065971263342, decay.Decay(GetTimeDurationRep(90)));
  EXPECT_DOUBLE_EQ(0.4345982085070782, decay.Decay(GetTimeDurationRep(150)));
  EXPECT_DOUBLE_EQ(0.36787944117144233, decay.Decay(GetTimeDurationRep(180)));
  EXPECT_DOUBLE_EQ(0.26359713811572683, decay.Decay(GetTimeDurationRep(240)));
  EXPECT_DOUBLE_EQ(0.18887560283756183, decay.Decay(GetTimeDurationRep(300)));
  EXPECT_DOUBLE_EQ(0.1353352832366127, decay.Decay(GetTimeDurationRep(360)));
}

TEST(TimeDecay, HalfLifeDecayNinetyDays) {
  TimeDecay<integer_day, HalfLifeDecay> decay(HalfLifeDecay(90));

  EXPECT_DOUBLE_EQ(90, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(90, decay.HalfLife());

  EXPECT_DOUBLE_EQ(0.89089871814033927, decay.Decay(GetTimeDuration(15)));
  EXPECT_DOUBLE_EQ(0.79370052598409979, decay.Decay(GetTimeDuration(30)));
  EXPECT_DOUBLE_EQ(0.6299605249474366, decay.Decay(GetTimeDuration(60)));
  EXPECT_DOUBLE_EQ(0.5, decay.Decay(GetTimeDuration(90)));
  EXPECT_DOUBLE_EQ(0.3149802624737183, decay.Decay(GetTimeDuration(150)));
  EXPECT_DOUBLE_EQ(0.25, decay.Decay(GetTimeDuration(180)));
  EXPECT_DOUBLE_EQ(0.0625, decay.Decay(GetTimeDuration(360)));
}

TEST(TimeDecay, HalfLifeDecayOneEightyDays) {
  TimeDecay<integer_day, HalfLifeDecay> decay(HalfLifeDecay(180));

  EXPECT_DOUBLE_EQ(180, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(180, decay.HalfLife());

  EXPECT_DOUBLE_EQ(0.94387431268169353, decay.Decay(GetTimeDuration(15)));
  EXPECT_DOUBLE_EQ(0.89089871814033927, decay.Decay(GetTimeDuration(30)));
  EXPECT_DOUBLE_EQ(0.79370052598409979, decay.Decay(GetTimeDuration(60)));
  EXPECT_DOUBLE_EQ(0.70710678118654757, decay.Decay(GetTimeDuration(90)));
  EXPECT_DOUBLE_EQ(0.56123102415468651, decay.Decay(GetTimeDuration(150)));
  EXPECT_DOUBLE_EQ(0.5, decay.Decay(GetTimeDuration(180)));
  EXPECT_DOUBLE_EQ(0.3968502629920499, decay.Decay(GetTimeDuration(240)));
  EXPECT_DOUBLE_EQ(0.3149802624737183, decay.Decay(GetTimeDuration(300)));
  EXPECT_DOUBLE_EQ(0.25, decay.Decay(GetTimeDuration(360)));
}

}  // namespace test
}  // namespace decay
