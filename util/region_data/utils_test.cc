// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "util/region_data/utils.h"

#include "test/cc/test_main.h"
#include "util/serial/serializer.h"

namespace region_data {
namespace utils {
namespace test {

TEST(NormalizeString, Sanity) {
  EXPECT_EQ("q asd asd 0 12 asd", NormalizeString("@Q#$ASD()$ASD 0 12)()()(   ASD#@$%@  "));
  EXPECT_EQ("aaaa cc eeee oo y r", NormalizeString(" aAäá çč eéÉE öÖ ý ř "));
  EXPECT_EQ("aa ee ii oo uu", NormalizeString(" aA@ eE$ )iI oO uU "));
  EXPECT_EQ("https www room77 com hotel harvey sun suites of new orleans harvey 205047",
      NormalizeString(
          "https://www.room77.com/hotel-harvey-sun-suites-of-new-orleans-harvey   -205047"));
  {
    string str;
    serial::Serializer::FromJSON("\"Alla Citt\u00C3\u00A0 di Trieste\"", &str);
    EXPECT_EQ("alla citta di trieste", NormalizeString(str));
  }
  {
    string str;
    serial::Serializer::FromJSON("\"Sun Suites of New Orleans-Harvey\u00A0\u00A0\u00A0\"", &str);
    EXPECT_EQ("sun suites of new orleans harvey", NormalizeString(str));
  }

  EXPECT_EQ("kyoto下京区", NormalizeString("Kyoto下京区"));
}

}  // namespace test
}  // namespace utils
}  // namespace region_data
