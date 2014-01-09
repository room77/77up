//
// Copyright 2007 OpTrip, Inc.
//
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "util/network/netclient.h"
#include "util/network/netserver.h"
#include "test/cc/test_main.h"

FLAG_string(server, "localhost", "echo server hostname");
FLAG_int(port, 11111, "echo server port number");

namespace test {

class EchoServer : public NetServer {
 private:
  // Incoming request is complete if it contains a newline character.
  virtual bool RequestIsComplete(const string& r,
                                 unsigned int *request_size) const {
    const char *s = r.c_str();
    const char *loc = strchr(s, '\n');
    if (loc == NULL)
      return false;
    else {
      *request_size = loc - s + 1;
      return true;
    }
  }

  // Allow multiple requests per connection.
  virtual bool OneRequestPerConnection() const { return false; }

  // Process a request by echoing it.
  virtual string ProcessRequest(const string& request,
                                bool *keep_alive,
                                const tConnectionInfo *connection) {
    *keep_alive = true;
    LOG(INFO) << request;
    return request;
  }
};

class EchoClient : public NetClient {
 public:
  EchoClient() {};
  virtual ~EchoClient() {};
 private:
  // incoming reply data is complete if it contains newline character
  virtual bool ReplyIsComplete() const {
    return (strchr(reply_.c_str(), '\n') != NULL);
  }
};

class EchoClientServerTest : public testing::Test {
 public:
  static void SetUpTestCase() {
    ASSERT_GT(gFlag_port , 0) << "Port number must be positive!";
    mutex m;
    unique_lock<mutex> lock(m);
    condition_variable cond_var;
    server_thread_.reset(new thread(bind(
        &EchoClientServerTest::AsyncStartServer, &m, &cond_var)));
    cond_var.wait(lock);
  }

  static void TearDownTestCase() {
    server_->set_prepare_shutdown(true);
    server_thread_->join();
  }

  static void AsyncStartServer(mutex* m, condition_variable* cond_var) {
    server_.reset(new EchoServer());
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
    client_.reset(new EchoClient());
    ASSERT_TRUE(client_->EstablishConnection(gFlag_server, gFlag_port))
        << "Unable to establish connection.";
  }

  virtual void TearDown() {
    ASSERT_TRUE(client_->CloseConnection());
  }

  static unique_ptr<EchoServer> server_;
  static unique_ptr<thread> server_thread_;
  unique_ptr<EchoClient> client_;
};

unique_ptr<EchoServer> EchoClientServerTest::server_;
unique_ptr<thread> EchoClientServerTest::server_thread_;

TEST_F(EchoClientServerTest, Sanity) {
  const string str = "hello world\n";
  EXPECT_TRUE(client_->SendMessage(str));
  EXPECT_TRUE(client_->WaitForReply());
  EXPECT_EQ(str, client_->get_reply());

}

}  // namespace test
