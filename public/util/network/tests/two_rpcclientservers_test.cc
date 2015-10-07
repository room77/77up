//
// Copyright 2007 OpTrip, Inc.
//
// This test addresses the bug where two RPC clients from the same host could
// not be created due to a bug in the dns resolver.

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "util/network/rpcclient.h"
#include "util/network/rpcserver.h"
#include "util/network/method/server_method.h"
#include "util/network/tests/rpcstruct_test.h"
#include "test/cc/test_main.h"

FLAG_string(server, "localhost", "server hostname.");
FLAG_int(port1, 11113, "server port for server 1.");
FLAG_int(port2, 11114, "server port for server 2.");

namespace network {
namespace test {

const char server1[] = "server1";
const char server2[] = "server2";

// RPC function definitions
namespace rpc_func {

// function 1: addition
class Addition : public ServerMethod {
 public:
  string operator()(const tInput& input, int *output) {
    *output = input.a + input.b;
    VLOG(2) << "calculating: "
           << input.a << " + " << input.b << " -> " << *output;

    string ip;
    GetCallerIP(&ip);
    VLOG(2) << "caller is " << ip;

    return "";  // success
  }
};
ServerMethodRegister<tInput, int, Addition> reg_Addition(
    server1, "PLUS", {1, 1});

// function 2: subtraction
class Subtraction : public ServerMethod {
 public:
  string operator()(const tInput& input, int *output) {
    *output = input.a - input.b;
    VLOG(2) << "calculating: "
           << input.a << " - " << input.b << " -> " << *output;
    return "";  // success
  }
};
ServerMethodRegister<tInput, int, Subtraction> reg_Subtraction(
    server2, "MINUS", {1, 1});

}  // namespace rpc_func

class TwoRPCClientServersTest : public testing::Test {
 public:
  static void SetUpTestCase() {
    ASSERT_GT(gFlag_port1 , 0) << "Port number 1 must be positive!";
    ASSERT_GT(gFlag_port2 , 0) << "Port number 2 must be positive!";

    mutex m1, m2;
    unique_lock<mutex> lock1(m1), lock2(m2);
    condition_variable cond_var1, cond_var2;
    server_threads_.push_back(shared_ptr<thread>(new thread(bind(
        &TwoRPCClientServersTest::AsyncStartServer, 0, &m1, &cond_var1))));
    server_threads_.push_back(shared_ptr<thread>(new thread(bind(
        &TwoRPCClientServersTest::AsyncStartServer, 1, &m2, &cond_var2))));
    cond_var1.wait(lock1);
    cond_var2.wait(lock2);
  }

  static void TearDownTestCase() {
    for (shared_ptr<RPCServer>& server : servers_)
      server->set_prepare_shutdown(true);

    LOG(INFO) << "Waiting for server threads to join.";
    for (shared_ptr<thread>& server_thread : server_threads_)
      server_thread->join();
    LOG(INFO) << "Server teardown complete.";
  }

  static void AsyncStartServer(int id, mutex* m, condition_variable* cond_var) {
    ASSERT(id == 1 || id == 0);
    shared_ptr<RPCServer> server(new RPCServer(id == 0 ? server1 : server2));
    int port = id == 0 ? gFlag_port1 : gFlag_port2;
    server->set_portnum(port);
    server->set_return_on_shutdown(true);
    server->PrepareServer();
    {
      lock_guard<mutex> l(m_);
      servers_.push_back(server);
    }
    {
      unique_lock<mutex> lock(*m);
      cond_var->notify_all();
    }
    server->StartServer();
  }

 protected:
  static vector<shared_ptr<RPCServer> > servers_;
  static vector<shared_ptr<thread> > server_threads_;
  static mutex m_;
};

vector<shared_ptr<RPCServer> > TwoRPCClientServersTest::servers_;
vector<shared_ptr<thread> > TwoRPCClientServersTest::server_threads_;
mutex TwoRPCClientServersTest::m_;

TEST_F(TwoRPCClientServersTest, Sanity) {
  tInput input = {1, 10};

  RPCClient client1, client2;
  ASSERT_TRUE(client1.EstablishConnection(gFlag_server, gFlag_port1));

  int output = 0;
  EXPECT_EQ("", client1.CallWithoutCookie("PLUS", input, &output));
  EXPECT_EQ(11, output);

  ASSERT_TRUE(client2.EstablishConnection(gFlag_server, gFlag_port2));
  output = 0;
  EXPECT_EQ("", client2.CallWithoutCookie("MINUS", input, &output));
  EXPECT_EQ(-9, output);

  ASSERT_TRUE(client1.CloseConnection());
  ASSERT_TRUE(client2.CloseConnection());
}

}  // namespace test
}  // namespace network
