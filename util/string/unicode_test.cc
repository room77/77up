// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

#include <deque>
#include <future>
#include "unicode.h"
#include "test/cc/test_main.h"

TEST(RemoveAccent, Sanity) {
  EXPECT_EQ(" aAaa cc eeEE oO y r ",
            unicode::RemoveAccent()(" aAäá çč eéÉE öÖ ý ř "));
}

TEST(Lower, Sanity) {
  EXPECT_EQ(" aaäá çč eéée öö ý ř ", unicode::Lower()(" aAäá çč eéÉE öÖ ý ř "));
  EXPECT_EQ(" aa ee ii oo uu ", unicode::Lower()(" aA eE iI oO uU "));
}

TEST(Upper, Sanity) {
  EXPECT_EQ(" AAÄÁ ÇČ EÉÉE ÖÖ Ý Ř ", unicode::Upper()(" aAäá çč eéÉE öÖ ý ř "));
  EXPECT_EQ(" AA EE II OO UU ", unicode::Upper()(" aA eE iI oO uU "));
}

TEST(LowerRemoveAccent, Sanity) {
  EXPECT_EQ(" aaaa cc eeee oo y r ",
            unicode::LowerRemoveAccent()(" aAäá çč eéÉE öÖ ý ř "));
  EXPECT_EQ(" aa ee ii oo uu ",
            unicode::LowerRemoveAccent()(" aA eE iI oO uU "));
}

TEST(UpperRemoveAccent, Sanity) {
  EXPECT_EQ(" AAAA CC EEEE OO Y R ",
            unicode::UpperRemoveAccent()(" aAäá çč eéÉE öÖ ý ř "));
  EXPECT_EQ(" AA EE II OO UU ",
            unicode::UpperRemoveAccent()(" aA eE iI oO uU "));
}

TEST(NormalizeForIndexing, Sanity) {
  EXPECT_EQ(" aaaa cc eeee oo y r ",
            unicode::NormalizeForIndexing()(" aAäá çč eéÉE öÖ ý ř "));
  EXPECT_EQ(" aa ee ii oo uu ",
            unicode::NormalizeForIndexing()(" aA eE iI oO uU "));
}

TEST(CanonicalString, Sanity) {
  EXPECT_EQ(unicode::Canonicalize()("AbcDEf012-_+"), "abcdef012 _ ");
}

TEST(Utf8ToUtf32, Test) {
  unicode::Convert<string, wstring> utf8_to_utf32;
  string s = " aaäá çč eéée öö ý ř ";
  wstring ws = utf8_to_utf32(s);
  EXPECT_EQ(s.size(), 31);
  EXPECT_EQ(ws.size(), 21);
}

TEST(MultiThreadedStressTest, Test) {
  unicode::Lower lower;
  unicode::Upper upper;
  unicode::LowerRemoveAccent lower_remove_accent;
  unicode::UpperRemoveAccent upper_remove_accent;
  unicode::NormalizeForIndexing normalize_for_indexing;
  unicode::Convert<string, wstring> utf8_to_utf32;
  deque<future<bool>> futures;
  for (int i = 0; i < 10; ++i) {
    futures.push_back(async(launch::async, [&]{
      bool ret = true;
      for (int i = 0; i < 10000; ++i) {
        ret &= lower(" aAäá çč eéÉE öÖ ý ř ") == " aaäá çč eéée öö ý ř ";
        ret &= upper(" aAäá çč eéÉE öÖ ý ř ") == " AAÄÁ ÇČ EÉÉE ÖÖ Ý Ř ";
        ret &= lower_remove_accent(" aAäá çč eéÉE öÖ ý ř ") == " aaaa cc eeee oo y r ";
        ret &= upper_remove_accent(" aAäá çč eéÉE öÖ ý ř ") == " AAAA CC EEEE OO Y R ";
        ret &= normalize_for_indexing(" aAäá çč eéÉE öÖ ý ř ") == " aaaa cc eeee oo y r ";
        ret &= normalize_for_indexing(" aA eE iI oO uU ") == " aa ee ii oo uu ";
        ret &= utf8_to_utf32(" aaäá çč eéée öö ý ř ").size() == 21;
      }
      return ret;
    }));
  }
  for (auto& f : futures) EXPECT_TRUE(f.get());
}
