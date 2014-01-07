// Copyright 2013 Room77, Inc.
// Author: oztekin@room77.com (Uygar Oztekin), karepker@room77.com (Kar Epker)

// Test cases for compression functions

#include "gzip.h"
#include "test/cc/unit_test.h"

FLAG_string(test_path, "util/testdata/", "The path to the test data files");

namespace Compression {
namespace test {

TEST(Compression, CompressDecompress) {
  string expected1 =
    "... being a Linux user is sort of like living in a house inhabited by "
     "a large family of carpenters and architects. Every morning when you "
     "wake up, the house is a little different. Maybe there is a new turret, "
     "or some walls have moved. Or perhaps someone has temporarily removed "
     "the floor under your bed. - Unix for Dummies, 2nd Edition";
  string compressed = Compression::GzipCompress(expected1);
  string decompressed = Compression::GzipDecompress(compressed);
  EXPECT_EQ(decompressed, expected1);

  string expected2 = "";
  compressed = Compression::GzipCompress(expected2);
  decompressed = Compression::GzipDecompress(compressed);
  EXPECT_EQ(decompressed, expected2);

  string expected3 = "12345";
  compressed = Compression::GzipCompress(expected3);
  decompressed = Compression::GzipDecompress(compressed);
  EXPECT_EQ(decompressed, expected3);
}

TEST(Compression, LineCompression) {
  string expected = "I'm Bane, yes, that's my name.\n"
    "When you hear the name Bane, I guarantee the pain.\n"
    "I'm coming after you, Bruce Wayne.\n"
    "I'm stronger, smarter, clinically insane.\n\n"
    "I'm Bane, yes, that's my name.\n"
    "Bruce Wayne and the Batman are totally the same.\n"
    "I broke his back mortal comBAT smack,\n"
    "then I cracked my '28 Krug champagne.\n\n"
    "I'm Bane, yes, it's a shame.\n"
    "I declare martial law, and you all complain!\n"
    "I laugh when you ask why I wear the mask.\n"
    "I'll explain. It's because ...\n\n"
    "I'm Bane, yes, that's my name.\n"
    "You say it too much, the name becomes inane.\n"
    "Of course! Some think my plan lacks gain.\n"
    "If you say it to my face, I'll crash your plane.\n\n"
    "--Auralnauts Dark Knight Rises outtakes\n";
  // http://www.youtube.com/watch?v=fLFAXvFYhsE

  string concatenated;
  Compression::ProcessGzipLines(gFlag_test_path + "compression_test_given.txt.gz",
      [&concatenated](const string& input) -> bool {
    concatenated.append(input);
    return input.size();
  });
  EXPECT_EQ(concatenated, expected);
}

}  // namespace Test
}  // namespace Compression
