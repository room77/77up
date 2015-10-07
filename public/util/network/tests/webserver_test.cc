//
// Copyright 2007 OpTrip, Inc.
//

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "util/network/webserver.h"

#include "util/network/httpclient.h"
#include "test/cc/test_main.h"

FLAG_string(server, "localhost", "server hostname.");
FLAG_int(port, 11115, "server port number");

namespace test {

// A simple web server that always displays the current time.
class TestWebServer : public WebServer {
 public:
  virtual ~TestWebServer() {};

  virtual int ProcessHttpRequest(const string& url,
                                 bool is_post,
                                 const string& cgi_arguments,
                                 const string& input_cookie,
                                 const string& referrer,
                                 // return values below
                                 string *result_content,
                                 string *content_type,
                                 int *max_cache_seconds) {
    ASSERT(result_content != NULL);
    ASSERT(content_type != NULL);
    ASSERT(max_cache_seconds != NULL);

    char time_buf[32];
    time_t t = time(NULL);
    ctime_r(&t, time_buf);

    *result_content =
      string("<html><head><title>A simple server</title></head>\n") +
      "<body>The current time is: " + time_buf + "...<p>\n" +
      "You requested: " + strutil::EscapeString_HTML(url) + "<p>\n" +
      (is_post ? "using POST" : "using GET") + "<p>\n" +
      "with arguments: " + strutil::EscapeString_HTML(cgi_arguments) +
      "<p>\n</body></html>\n";
    *content_type = "text/html";
    *max_cache_seconds = 0;
    return WebServer::gHttpResponse_OK;
  }
};

class WebServerTest : public testing::Test {
 public:
  static void SetUpTestCase() {
    ASSERT_GT(gFlag_port , 0) << "Port number must be positive!";
    mutex m;
    unique_lock<mutex> lock(m);
    condition_variable cond_var;
    server_thread_.reset(new thread(bind(
        &WebServerTest::AsyncStartServer, &m, &cond_var)));
    cond_var.wait(lock);
  }

  static void TearDownTestCase() {
    server_->set_prepare_shutdown(true);
    server_thread_->join();
  }

  static void AsyncStartServer(mutex* m, condition_variable* cond_var) {
    server_.reset(new TestWebServer());
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
    client_.reset(new HttpClient());
  }

  static unique_ptr<TestWebServer> server_;
  static unique_ptr<thread> server_thread_;
  unique_ptr<HttpClient> client_;
};

unique_ptr<TestWebServer> WebServerTest::server_;
unique_ptr<thread> WebServerTest::server_thread_;

// Test special RPC message wrapped in HTTP POST.
TEST_F(WebServerTest, SanityGet) {
  string output;
  int status;
  EXPECT_TRUE(client_->HttpGet(gFlag_server, gFlag_port, "/", &status, &output,
                               NULL));
  LOG(INFO) << "Msg: [" << output << "]";
  EXPECT_NE(string::npos, output.find("A simple server"));
  EXPECT_NE(string::npos, output.find("The current time is:"));
  EXPECT_NE(string::npos, output.find("using GET"));
}

TEST_F(WebServerTest, SanityPost) {
  string output;
  int status;
  EXPECT_TRUE(client_->HttpPost(gFlag_server, gFlag_port, "/", "", &status,
                                &output, NULL));
  LOG(INFO) << "Msg: [" << output << "]";
  EXPECT_NE(string::npos, output.find("A simple server"));
  EXPECT_NE(string::npos, output.find("The current time is:"));
  EXPECT_NE(string::npos, output.find("using POST"));
}

}  // namespace test
