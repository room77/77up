#include "util/file/filereader.h"

#include "base/common.h"
#include "test/cc/test_main.h"
#include "util/serial/serializer.h"

FLAG_string(test_strings_file,
            "util/file/testdata/strings.txt",
            "File with strings for testing filereader");

FLAG_string(test_structs_file,
            "util/file/testdata/structs.txt",
            "File with structs for testing filereader");

namespace test {

TEST(FileReaderTest, ReadAllAs_string) {
  unordered_set<string> list = FileReader::ReadAllAs<unordered_set<string> >(
      gFlag_test_strings_file, FileReader::StringLineParser());
  //LOG(INFO) << *list.begin();
  ASSERT_EQ(list.size(), 5);
}

// make sure the protobufs are reading in the proper amount of messages
TEST(FileReaderTest, ProcessAll_struct) {
  struct tTest {
    int i;
    SERIALIZE(i*1);
  };
  /*
  // generate test file
  ofstream f(gFlag_test_structs_file);
  for (int i = 0 ; i < 10 ; ++i) {
    tTest test;
    test.i = i;
    f << test.ToBinary();
  }
  f.close();
  */
  auto callback = [](const tTest& t) {};
  ASSERT_EQ(FileReader::ProcessAll<tTest>(gFlag_test_structs_file, callback), 10);
}
} // namespace test
