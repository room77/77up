// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "file.h"

#include "test/cc/test_main.h"

namespace file {
namespace test {

TEST(Exists, Sanity) {
  EXPECT_TRUE(Exists("base"));
  EXPECT_TRUE(Exists("base/RULES"));
}

TEST(IsFile, Sanity) {
  EXPECT_FALSE(IsFile("base"));
  EXPECT_TRUE(IsFile("base/RULES"));
}

TEST(IsDirectory, Sanity) {
  EXPECT_TRUE(IsDirectory("base"));
  EXPECT_FALSE(IsDirectory("base/RULES"));
}

TEST(GetExtension, Sanity) {
  EXPECT_EQ("", GetExtension("file"));
  EXPECT_EQ(".ext", GetExtension("file.abc.ext"));
  EXPECT_EQ(".ext", GetExtension("dir/abc.ext"));
  EXPECT_EQ(".", GetExtension("abc."));
}

TEST(HasExtension, Sanity) {
  EXPECT_FALSE(HasExtension("ext", ".ext"));
  EXPECT_TRUE(HasExtension(".ext", ".ext"));
  EXPECT_FALSE(HasExtension(".ext", ".abc"));
  EXPECT_TRUE(HasExtension("abc.ext", ".ext"));
  EXPECT_FALSE(HasExtension("abc.ext", ".abc"));
  EXPECT_TRUE(HasExtension("dir/abc.ext", ".ext"));
  EXPECT_FALSE(HasExtension("dir/abc.ext", ".abc"));
  EXPECT_TRUE(HasExtension("dir.abc.ext", ".ext"));
  EXPECT_FALSE(HasExtension("dir.abc.ext", ".abc"));
}

TEST(ReplaceExtension, Sanity) {
  EXPECT_EQ("abc.new", ReplaceExtension("abc.old", ".new"));
  EXPECT_EQ("abc.new", ReplaceExtension("abc.", ".new"));
  EXPECT_EQ("abc.new", ReplaceExtension("abc", ".new"));
  EXPECT_EQ("abc.", ReplaceExtension("abc.old", "."));

  EXPECT_EQ(".new", ReplaceExtension(".old", ".new"));
  EXPECT_EQ(".new", ReplaceExtension("", ".new"));

  EXPECT_EQ("dir/abc.new", ReplaceExtension("dir/abc.old", ".new"));
  EXPECT_EQ("dir/abc.new", ReplaceExtension("dir/abc.", ".new"));
  EXPECT_EQ("dir/abc.", ReplaceExtension("dir/abc.old", "."));

  EXPECT_EQ("dir.abc.new", ReplaceExtension("dir.abc.old", ".new"));
  EXPECT_EQ("dir.abc.new", ReplaceExtension("dir.abc.", ".new"));
  EXPECT_EQ("dir.abc.", ReplaceExtension("dir.abc.old", "."));
}

}  // namespace test
}  // namespace File
