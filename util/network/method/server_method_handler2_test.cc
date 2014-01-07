// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/network/method/server_method_handler.h"

#include "util/network/tests/rpcstruct_test.h"
#include "util/network/method/server_method.h"
#include "test/cc/unit_test.h"

namespace network {
namespace test {

struct Addition : public tServerMethod<tInput, tOutput> {
  virtual string operator()(const tInput& input, tOutput *output, vector<string>* outCookies) const override {
    output->i = input.a + input.b;
    return "";  // success
  }
  virtual void Example(tInput *req) const override {
    req->a = 1;
    req->b = 2;
  }
};
tServerMethodRegister<Addition> reg_add_test1("test_server1,test_server2", "ADD");

struct Subtraction : public tServerMethod<tInput, tOutput> {
  string operator()(const tInput& input, tOutput *output, vector<string>* outCookies) const override {
    output->i = input.a - input.b;
    return "";  // success
  }
  void Example(tInput *req) const override {
    req->a = 11;
    req->b = 12;
  }
};

tServerMethodRegister<Subtraction> reg_sub_test1("test_server1", "SUB");

TEST(ServerMethodHandlerCollection, Sanity) {

  tServerMethodRegister<Subtraction> reg_sub_test3("test_server3", "SUB");
  tServerMethodRegister<Addition>    reg_add_test2("test_server4", "ADD");

  ServerMethodHandlerCollection::shared_proxy proxy1 =
      ServerMethodHandlerCollection::make_shared("test_server1");
  ASSERT_NOTNULL(proxy1);

  EXPECT_EQ(2, proxy1->size());
  EXPECT_TRUE(nullptr != proxy1->GetHandler("ADD"));
  EXPECT_TRUE(nullptr != proxy1->GetHandler("SUB"));

  ServerMethodHandlerCollection::shared_proxy proxy2 =
      ServerMethodHandlerCollection::make_shared("test_server2");
  ASSERT_NOTNULL(proxy2);

  EXPECT_EQ(1, proxy2->size());
  EXPECT_TRUE(nullptr != proxy1->GetHandler("ADD"));

  ServerMethodHandlerCollection::shared_proxy proxy3 =
      ServerMethodHandlerCollection::make_shared("test_server3");
  ASSERT_NOTNULL(proxy3);

  EXPECT_EQ(1, proxy3->size());
  EXPECT_TRUE(nullptr != proxy3->GetHandler("SUB"));

  ServerMethodHandlerCollection::shared_proxy proxy4 =
      ServerMethodHandlerCollection::make_shared("test_server4");
  ASSERT_NOTNULL(proxy4);

  EXPECT_EQ(1, proxy4->size());
  EXPECT_TRUE(nullptr != proxy4->GetHandler("ADD"));
}

TEST(ServerMethodHandlerCollection, DuplicateOpName) {
  typedef tServerMethodRegister<Subtraction> SubReg;
  ASSERT_DEATH(SubReg("test_server1", "ADD"), "");
}

}  // namespace test
}  // namespace network
