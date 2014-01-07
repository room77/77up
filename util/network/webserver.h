//
// Copyright 2007 OpTrip, Inc.
//

#ifndef _PUBLIC_UTIL_NETWORK_WEBSERVER_H_
#define _PUBLIC_UTIL_NETWORK_WEBSERVER_H_

#include "util/network/netserver.h"

// a simple web server with limited functionality
//
// This web server always returns the same message regardless of input.
// To make a real, useful web server, derive a class from WebServer and
// override virtual function ProcessHttpRequest()

class WebServer : public NetServer {
 public:
  virtual ~WebServer() {};

  // supported return values:
  const static int gHttpResponse_OK          = 200;
  const static int gHttpResponse_NotFound    = 404;
  const static int gHttpResponse_ServerError = 500;
  const static int gHttpResponse_ServerBusy  = 503;

  // ProcessHttpRequest:
  //   process the given request; return one of the codes above
  virtual int ProcessHttpRequest(const string& url,
                                 bool is_post,
                                 const string& cgi_arguments,
                                 const string& input_cookie,
                                 const string& referrer,
                                 // return values below
                                 string *result_content,
                                 string *content_type,
                                 int *max_cache_seconds);

  // check if all bytes have been received in an http request
  static bool HttpRequestIsComplete(const string& r,
                                    unsigned int *request_size,
                                    string *url,  // may be NULL
                                    string *cgi_arguments,  // may be NULL
                                    string *input_cookie,   // may be NULL
                                    string *referrer,       // may be NULL
                                    bool *accept_gzip,  // may be NULL
                                    bool *keep_alive,   // may be NULL
                                    bool *is_post,      // may be NULL
                                    unordered_map<string, string> *http_header);

  // assemble http response from components
  // (note: in case response_code is not gHttpResponse_OK, all other arguments
  //        are ignored)
  static string ConstructHttpResponse(int response_code,
                                      const string& result_content,
                                      const string& content_type,
                                      bool client_accepts_gzip,
                                      const vector<string>& full_cookie_specs,
                                      int max_cache_seconds);

  // return a summary string for logging purposes
  static string HttpRequestSummary(const string& url,
                                   const string& cgi_arguments,
                                   bool is_post);

  // guess content type from file name suffix
  static string GuessContentType(const string& filename);

 private:
  // check if all bytes have been received
  virtual bool RequestIsComplete(const string& r,
                                 unsigned int *request_size) const {
    return HttpRequestIsComplete(r, request_size,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL);
  }

  // allow multiple requests per connection
  virtual bool OneRequestPerConnection() const { return false; }

  // process a request
  virtual string ProcessRequest(const string& request,
                                bool *keep_alive,
                                const tConnectionInfo *connection);
};

#endif  // _PUBLIC_UTIL_NETWORK_WEBSERVER_H_
