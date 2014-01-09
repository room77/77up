// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/network/method/server_method.h"

#include "test/cc/test_main.h"

#include "util/templates/container_util.h"

namespace network {
namespace test {

class ExampleServerMethod1 : public ServerMethod {
  string operator()(const int& input, int *output) {
    return "1";
  }
};

class ExampleServerMethod2 : public ServerMethod {
  string operator()(const int& input, int *output) {
    return "2";
  }
};

// Test class for MyTest.
class ServerMethodTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    PrepareServerMethod(&server_method_1_);
  }

  void PrepareServerMethod(ServerMethod* server_method) {
    server_method->SetReferrer("https://url.com");

    server_method->ParseInputCookie("c1=v1;c2=v2");
    server_method->set_arg_map({{"arg1", "1"}, {"arg2", "true"}});
    server_method->set_http_header({{"Connection", "keep-alive"}, {"Accept", "*/*"}});
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  ExampleServerMethod1 server_method_1_;
  ExampleServerMethod2 server_method_2_;
};

TEST_F(ServerMethodTest, Sanity) {
  EXPECT_EQ("https://url.com", server_method_1_.referrer());

  EXPECT_EQ("v1", server_method_1_.GetCookie("c1"));
  EXPECT_EQ("v2", server_method_1_.GetCookie("c2"));
  EXPECT_EQ("", server_method_1_.GetCookie("invalid"));

  // A new session id must be defined for each server method if not already set.
  EXPECT_TRUE(server_method_1_.GetSessionId().size());

  EXPECT_EQ("1", ::util::tl::FindWithDefault(server_method_1_.arg_map(), "arg1", ""));
  EXPECT_EQ("true", ::util::tl::FindWithDefault(server_method_1_.arg_map(), "arg2", ""));
  EXPECT_EQ("", ::util::tl::FindWithDefault(server_method_1_.arg_map(), "invalid", ""));

  EXPECT_EQ("keep-alive",
            ::util::tl::FindWithDefault(server_method_1_.http_header(), "Connection", ""));
  EXPECT_EQ("*/*", ::util::tl::FindWithDefault(server_method_1_.http_header(), "Accept", ""));
  EXPECT_EQ("", ::util::tl::FindWithDefault(server_method_1_.http_header(), "invalid", ""));
}

TEST_F(ServerMethodTest, CannotOverrideQinArgMap) {
  server_method_1_.set_arg_map({{"arg1", "2"}, {"q", "abc"}});

  // q is not allowed to be overwritten.
  EXPECT_EQ("", ::util::tl::FindWithDefault(server_method_1_.arg_map(), "q", ""));
  EXPECT_EQ("2", ::util::tl::FindWithDefault(server_method_1_.arg_map(), "arg1", ""));
  EXPECT_EQ("true", ::util::tl::FindWithDefault(server_method_1_.arg_map(), "arg2", ""));
  EXPECT_EQ("", ::util::tl::FindWithDefault(server_method_1_.arg_map(), "invalid", ""));
}

TEST_F(ServerMethodTest, Cascade) {
  server_method_2_.Cascade(&server_method_1_);
  EXPECT_EQ("https://url.com", server_method_2_.referrer());

  EXPECT_EQ("v1", server_method_2_.GetCookie("c1"));
  EXPECT_EQ("v2", server_method_2_.GetCookie("c2"));
  EXPECT_EQ("", server_method_2_.GetCookie("invalid"));

  // A new session id must be defined for each server method if not already set.
  EXPECT_TRUE(server_method_2_.GetSessionId().size());
  EXPECT_EQ(server_method_1_.GetSessionId(), server_method_2_.GetSessionId());

  EXPECT_EQ("1", ::util::tl::FindWithDefault(server_method_2_.arg_map(), "arg1", ""));
  EXPECT_EQ("true", ::util::tl::FindWithDefault(server_method_2_.arg_map(), "arg2", ""));
  EXPECT_EQ("", ::util::tl::FindWithDefault(server_method_2_.arg_map(), "invalid", ""));

  EXPECT_EQ("keep-alive",
            ::util::tl::FindWithDefault(server_method_2_.http_header(), "Connection", ""));
  EXPECT_EQ("*/*", ::util::tl::FindWithDefault(server_method_2_.http_header(), "Accept", ""));
  EXPECT_EQ("", ::util::tl::FindWithDefault(server_method_2_.http_header(), "invalid", ""));
}


}  // namespace test
}  // namespace network
