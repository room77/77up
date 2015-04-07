/*
Copyright 2014, Room 77, Inc.
@author sball

Test for webserver
*/

#include "test/cc/test_main.h"
#include "util/network/webserver.h"

// Header uses \r\n
TEST(WebserverTest, TestHttpRequestIsComplete_RNNewlines) {
  string request =
      "POST /_rpc HTTP/1.1\r\nUser-Agent: OpTripBot www.optrip.com\r\n"
      "Accept-Encoding: gzip\r\nContent-Length: 11\r\n"
      "Content-Type: text/xml; charset=utf-8\r\nConnection: Close\r\n"
      "Host: localhost\r\nX-Forwarded-For: 192.168.77.52\r\n\r\nTESTCONTENT";
  unsigned int request_size;

  EXPECT_TRUE(WebServer::HttpRequestIsComplete(
      request, &request_size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL));
}

// Header uses \n
TEST(WebserverTest, TestHttpRequestIsComplete_NNewlines) {
  string request =
      "POST /_rpc HTTP/1.1\nUser-Agent: OpTripBot www.optrip.com\n"
      "Accept-Encoding: gzip\nContent-Length: 11\n"
      "Content-Type: text/xml; charset=utf-8\nConnection: Close\n"
      "Host: localhost\nX-Forwarded-For: 192.168.77.52\n\nTESTCONTENT";
  unsigned int request_size;

  EXPECT_TRUE(WebServer::HttpRequestIsComplete(
      request, &request_size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL));
}

// Header ends with \r\n\r\n\n
TEST(WebserverTest, TestHttpRequestIsComplete_ConsecutiveNewlines) {
  string request =
      "POST /_rpc HTTP/1.1\r\nUser-Agent: OpTripBot www.optrip.com\r\n"
      "Accept-Encoding: gzip\r\nContent-Length: 12\r\n"
      "Content-Type: text/xml; charset=utf-8\r\nConnection: Close\r\n"
      "Host: localhost\r\nX-Forwarded-For: 192.168.77.52\r\n\r\n\nTESTCONTENT";
  unsigned int request_size;

  EXPECT_TRUE(WebServer::HttpRequestIsComplete(
      request, &request_size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL));
}
