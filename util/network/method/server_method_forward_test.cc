// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/network/method/server_method_forward.h"

#include "test/cc/unit_test.h"

namespace network {
namespace test {

char local_server1[] = "local_server1";
char local_server2[] = "local_server2";


char remote_server1[] = "remote_server1";
char remote_server2[] = "remote_server2";

char remote_method1[] = "remote_method1";
char remote_method2[] = "remote_method2";
ServerMethodForwardRegister<int, int, remote_server1, remote_method1> reg1("local_server1", "local_method1", 1);
ServerMethodForwardRegister<int, int, remote_server2, remote_method1> reg2("local_server1", "local_method2", 1);
ServerMethodForwardRegister<int, int, remote_server1, remote_method1, 10> reg3("local_server2", "local_method1", 1);

TEST(ServerMethodForwardCollection, Sanity) {
  ServerMethodForwardRegister<int, int, remote_server1, remote_method2> reg("local_server1", "local_method3", 1);

  ServerMethodForwardCollection::shared_proxy fwd_proxy1 =
      ServerMethodForwardCollection::make_shared("local_server1");
  ASSERT_NOTNULL(fwd_proxy1);
  EXPECT_EQ(3, fwd_proxy1->size());
  EXPECT_TRUE(nullptr != fwd_proxy1->GetData("local_method1"));
  EXPECT_TRUE(nullptr != fwd_proxy1->GetData("local_method2"));
  EXPECT_TRUE(nullptr != fwd_proxy1->GetData("local_method3"));

  ServerMethodForwardCollection::shared_proxy fwd_proxy2 =
      ServerMethodForwardCollection::make_shared("local_server2");
  ASSERT_NOTNULL(fwd_proxy2);
  EXPECT_EQ(1, fwd_proxy2->size());
  EXPECT_TRUE(nullptr != fwd_proxy2->GetData("local_method1"));

  ServerMethodHandlerCollection::shared_proxy proxy1 =
      ServerMethodHandlerCollection::make_shared("local_server1");
  ASSERT_NOTNULL(proxy1);
  EXPECT_EQ(3, proxy1->size());
  EXPECT_TRUE(nullptr != proxy1->GetHandler("local_method1"));
  EXPECT_TRUE(nullptr != proxy1->GetHandler("local_method2"));
  EXPECT_TRUE(nullptr != proxy1->GetHandler("local_method3"));

  ServerMethodHandlerCollection::shared_proxy proxy2 =
      ServerMethodHandlerCollection::make_shared("local_server2");
  ASSERT_NOTNULL(proxy2);
  EXPECT_EQ(1, proxy2->size());
  EXPECT_TRUE(nullptr != proxy2->GetHandler("local_method1"));
}

TEST(ServerMethodHandlerCollection, DuplicateOP) {
  typedef ServerMethodForwardRegister<int, int, remote_server1, remote_method2,
      10> SubReg;
  ASSERT_DEATH(SubReg("local_server1", "local_method1", 1), "");
}

}  // namespace test
}  // namespace network
