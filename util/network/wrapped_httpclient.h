// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Wrapping the httpclient to make RPC or JSON requests.

#ifndef _PUBLIC_UTIL_NETWORK_WRAPPED_HTTPCLIENT_H_
#define _PUBLIC_UTIL_NETWORK_WRAPPED_HTTPCLIENT_H_

#include "util/network/httpclient.h"
#include "util/network/netclient.h"
#include "util/network/sslclient.h"
#include "util/network/rpc_datatypes.h"
#include "util/network/rpcclient.h"
#include "util/network/httputil.h"
#include "util/serial/serializer.h"

template <class NetBase>
class WrappedHttpBase : public HttpBase<NetBase> {
 public:
  virtual ~WrappedHttpBase() {}

  // Perform a RPC command wrapped around in a "POST"
  // command (R77's internal format).
  template<class tOutput>
    string RPCWrappedInHttpPost(const string& host, int port,
        const tServerRequestMessage& request, tOutput *output,
        string *output_cookie_full_spec_from_http = nullptr,
        string *output_cookie_full_spec_from_rpc = nullptr) {
    if (output_cookie_full_spec_from_http)
      output_cookie_full_spec_from_http->clear();
    if (output_cookie_full_spec_from_rpc)
      output_cookie_full_spec_from_rpc->clear();

    // Serialize the request, with size.
    string rpc_request = serial::Serializer::ToBinaryPrependSize(request);
    string rpc_response;
    HttpUtil::tHeaderType header;
    if (!Post(host, port, "/_rpc", rpc_request, &rpc_response,
              output_cookie_full_spec_from_http))
      return string("RPC call: ") + request.opname + " failed";

    return RPCClient::ParseRPCResponse(rpc_response, output,
                                       output_cookie_full_spec_from_rpc);
  }

  // open a connection, perform a RPC command wrapped around in a "POST"
  // command (R77's internal format) and close the connection
  template<class tInput, class tOutput>
    string RPCWrappedInHttpPost(const string& host, int port,
        const string& opname, const tInput& input, const string& input_cookie,
        const string& referrer, tOutput *output,
        string *output_cookie_full_spec_from_http = nullptr,
        string *output_cookie_full_spec_from_rpc = nullptr) {
    tServerRequestMessage raw_input;
    raw_input.opname = opname;
    raw_input.message = serial::Serializer::ToBinary(input);
    raw_input.cookie = input_cookie;
    raw_input.referrer = referrer;

    return RPCWrappedInHttpPost(host, port, raw_input, output,
                                output_cookie_full_spec_from_http,
                                output_cookie_full_spec_from_rpc);
  }

  // open a connection, perform a JSON command wrapped around in a "POST"
  // command (R77's internal format) and close the connection
  template<class tInput, class tOutput>
    string JSONWrappedInHttpPost(const string& host, int port,
        const string& opname, const tInput& input, const string& input_cookie,
        const string& referrer, const string& cgi_params, tOutput *output,
        string *output_cookie_full_spec_from_http = nullptr) {
    if (output_cookie_full_spec_from_http)
      output_cookie_full_spec_from_http->clear();

    if (!input_cookie.empty())
      this->AddHeader(string("Cookie: ") + input_cookie);

    if (!referrer.empty())
      this->AddHeader(string("Referer: ") + referrer);

    int status_code;
    string response;
    HttpUtil::tHeaderType header;
    if (!Post(host, port, "/" + opname,
              "q=" + serial::Serializer::ToJSON(input) + cgi_params,
              &response, output_cookie_full_spec_from_http))
      return string("JSON call: ") + opname + " failed";

    return serial::Serializer::FromJSON(response, output) ?
        "" : "Failed to parse JSON";
  }

 protected:
  // Returns true if the call succeeds, and false otherwise.
  bool Post(const string& host, int port, const string& path,
              const string& message, string *reply,
              string* output_cookie_full_spec_from_http) {
    bool res = false;
    int status_code = 0;
    HttpUtil::tHeaderType header;
    if (this->HttpPost(host, port, path, message, &status_code, reply,
                        &header)) {
      // TODO(edelman) - this only accepts one cookie being set.
      // need to accept multiple
      if (output_cookie_full_spec_from_http) {
        HttpUtil::tHeaderType::const_iterator i = header.find("Set-Cookie");
        if (i != header.end())
          *output_cookie_full_spec_from_http = i->second;
      }
      res = true;
    }
    return res;
  }
};

typedef WrappedHttpBase<NetClient> WrappedHttpClient;
typedef WrappedHttpBase<SSLClient> WrappedHttpsClient;

#endif  // _PUBLIC_UTIL_NETWORK_WRAPPED_HTTPCLIENT_H_
