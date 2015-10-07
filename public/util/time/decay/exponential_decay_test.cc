// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/time/decay/exponential_decay.h"

#include "test/cc/test_main.h"

namespace decay {
namespace test {

TEST(ExponentialDecayTest, NinetyDays) {
  ExponentialDecay decay(90);

  EXPECT_DOUBLE_EQ(90, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(62.383246250395075, decay.HalfLife());

  EXPECT_DOUBLE_EQ(0.84648172489061413, decay.Decay(15));
  EXPECT_DOUBLE_EQ(0.71653131057378927, decay.Decay(30));
  EXPECT_DOUBLE_EQ(0.51341711903259202, decay.Decay(60));
  EXPECT_DOUBLE_EQ(0.36787944117144233, decay.Decay(90));
  EXPECT_DOUBLE_EQ(0.18887560283756183, decay.Decay(150));
  EXPECT_DOUBLE_EQ(0.1353352832366127, decay.Decay(180));
  EXPECT_DOUBLE_EQ(0.018315638888734186, decay.Decay(360));
}

TEST(ExponentialDecayTest, OneEightyDays) {
  ExponentialDecay decay(180);

  EXPECT_DOUBLE_EQ(180, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(124.76649250079015, decay.HalfLife());

  EXPECT_DOUBLE_EQ(0.92004441462932329, decay.Decay(15));
  EXPECT_DOUBLE_EQ(0.84648172489061413, decay.Decay(30));
  EXPECT_DOUBLE_EQ(0.71653131057378927, decay.Decay(60));
  EXPECT_DOUBLE_EQ(0.60653065971263342, decay.Decay(90));
  EXPECT_DOUBLE_EQ(0.4345982085070782, decay.Decay(150));
  EXPECT_DOUBLE_EQ(0.36787944117144233, decay.Decay(180));
  EXPECT_DOUBLE_EQ(0.26359713811572683, decay.Decay(240));
  EXPECT_DOUBLE_EQ(0.18887560283756183, decay.Decay(300));
  EXPECT_DOUBLE_EQ(0.1353352832366127, decay.Decay(360));
}

TEST(HalfLifeDecay, NinetyDays) {
  HalfLifeDecay decay(90);

  EXPECT_DOUBLE_EQ(90, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(90, decay.HalfLife());

  EXPECT_DOUBLE_EQ(0.89089871814033927, decay.Decay(15));
  EXPECT_DOUBLE_EQ(0.79370052598409979, decay.Decay(30));
  EXPECT_DOUBLE_EQ(0.6299605249474366, decay.Decay(60));
  EXPECT_DOUBLE_EQ(0.5, decay.Decay(90));
  EXPECT_DOUBLE_EQ(0.3149802624737183, decay.Decay(150));
  EXPECT_DOUBLE_EQ(0.25, decay.Decay(180));
  EXPECT_DOUBLE_EQ(0.0625, decay.Decay(360));
}

TEST(HalfLifeDecay, OneEightyDays) {
  HalfLifeDecay decay(180);

  EXPECT_DOUBLE_EQ(180, decay.mean_life_time());
  EXPECT_DOUBLE_EQ(180, decay.HalfLife());

  EXPECT_DOUBLE_EQ(0.94387431268169353, decay.Decay(15));
  EXPECT_DOUBLE_EQ(0.89089871814033927, decay.Decay(30));
  EXPECT_DOUBLE_EQ(0.79370052598409979, decay.Decay(60));
  EXPECT_DOUBLE_EQ(0.70710678118654757, decay.Decay(90));
  EXPECT_DOUBLE_EQ(0.56123102415468651, decay.Decay(150));
  EXPECT_DOUBLE_EQ(0.5, decay.Decay(180));
  EXPECT_DOUBLE_EQ(0.3968502629920499, decay.Decay(240));
  EXPECT_DOUBLE_EQ(0.3149802624737183, decay.Decay(300));
  EXPECT_DOUBLE_EQ(0.25, decay.Decay(360));
}

}  // namespace test
}  // namespace decay
