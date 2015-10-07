//
// Copyright 2007 OpTrip, Inc.
//
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "util/network/rpcclient.h"
#include "util/network/rpcserver.h"
#include "util/network/wrapped_httpclient.h"
#include "util/network/method/server_method.h"
#include "util/network/tests/rpcstruct_test.h"
#include "test/cc/test_main.h"

FLAG_string(server, "localhost", "server hostname.");
FLAG_int(port, 11112, "server port number.");

using network::ServerMethod;
using network::RPCServer;

namespace test {

// RPC function definitions.
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

}  // namespace rpc_func

class RPCClientServerTest : public testing::Test {
 public:
  static void SetUpTestCase() {
    ASSERT_GT(gFlag_port , 0) << "Port number must be positive!";
    mutex m;
    unique_lock<mutex> lock(m);
    condition_variable cond_var;
    server_thread_.reset(new thread(bind(
        &RPCClientServerTest::AsyncStartServer, &m, &cond_var)));
    cond_var.wait(lock);
  }

  static void TearDownTestCase() {
    server_->set_prepare_shutdown(true);
    LOG(INFO) << "Waiting for server thread to join.";
    server_thread_->join();
    LOG(INFO) << "Server teardown complete.";
  }

  static void AsyncStartServer(mutex* m, condition_variable* cond_var) {
    server_.reset(new RPCServer());

    // A simple RPC server that performs addition and subtraction of two numbers.
    // Define sample input -- this is what's displayed via http debug interface.
    tInput sample_input;
    sample_input.a = 1;
    sample_input.b = 1;

    server_->RegisterHandler<tInput, int, rpc_func::Addition>(
        "PLUS", sample_input);
    server_->RegisterHandler<tInput, int, rpc_func::Subtraction>(
        "MINUS", sample_input);

    server_->set_portnum(gFlag_port);
    server_->set_return_on_shutdown(true);
    server_->PrepareServer();
    {
      unique_lock<mutex> lock(*m);
      cond_var->notify_one();
    }
    server_->StartServer();
  }

 protected:
  virtual void SetUp() {
    client_.reset(new RPCClient());
    client_->set_timeout(30);
    ASSERT_TRUE(client_->EstablishConnection(gFlag_server, gFlag_port))
        << "Unable to establish connection.";
  }

  virtual void TearDown() {
    ASSERT_TRUE(client_->CloseConnection());
  }


  static unique_ptr<RPCServer> server_;
  static unique_ptr<thread> server_thread_;
  unique_ptr<RPCClient> client_;
};

unique_ptr<RPCServer> RPCClientServerTest::server_;
unique_ptr<thread> RPCClientServerTest::server_thread_;

TEST_F(RPCClientServerTest, CallWithoutCookie) {
  tInput input = {1, 10};
  int output = 0;
  EXPECT_EQ("", client_->CallWithoutCookie("PLUS", input, &output));
  EXPECT_EQ(11, output);

  output = 0;
  EXPECT_EQ("", client_->CallWithoutCookie("MINUS", input, &output));
  EXPECT_EQ(-9, output);
}

// Test special RPC message wrapped in HTTP POST.
TEST_F(RPCClientServerTest, HTTPClient) {
  tInput input = {1, 10};
  int output = 0;

  WrappedHttpClient h;
  EXPECT_EQ("", h.RPCWrappedInHttpPost(gFlag_server, gFlag_port, "PLUS", input,
                                       "", "", &output, NULL, NULL));
  EXPECT_EQ(11, output);

  output = 0;
  EXPECT_EQ("", h.RPCWrappedInHttpPost(gFlag_server, gFlag_port, "MINUS", input,
                                       "", "", &output, NULL, NULL));
  EXPECT_EQ(-9, output);
}

}  // namespace test
