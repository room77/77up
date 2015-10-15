// Copyright 2015 Room77 Inc. All Rights Reserved.
// Author: sball@room77.com (Stephen Ball)

#ifndef _PUBLIC_UTIL_NETWORK_RPCCLIENT_UTIL_H_
#define _PUBLIC_UTIL_NETWORK_RPCCLIENT_UTIL_H_

#include "base/lite.h"
#include "util/codetranslator/server_config.h"
#include "util/network/rpc_datatypes.h"
#include "util/network/rpcclient.h"
#include "util/serial/serializer.h"

namespace network {

// Convenience functions for making RPC calls

// Standard template for a simple RPC call.
template<class tOutput>
string MakeRPCCall(const string& server_name,
                   const tServerRequestMessage& request, tOutput* output,
                   string* output_cookie = nullptr, int timeout_ms = -1) {
  RPCClient client;
  // TODO(edelman, pramodg, vkasera): Use clientpool to avoid frequent
  // connect/disconnect actions?
  // vkasera already has this somewhere in a local file.
  string err;
  static CodeTranslator::ServerConfig& server_config =
    CodeTranslator::ServerConfig::Instance();
  auto server = server_config.FindOneServer(server_name, 0);
  if (timeout_ms > 0) client.set_timeout_ms(timeout_ms);
  if (!client.EstablishConnection(server->host, server->tcp_port)) {
    err = "Cannot establish connection to " + server_name + "::" +
          request.opname;
    return err;
  }

  err = client.Call(request, output, output_cookie);

  if (!err.empty()) {
    LOG(INFO) << "Error msg from " << server_name << " request "
              << request.opname << ": " << err;
  }
  return err;
}

// Standard template for a simple RPC call.
template<class tInput, class tOutput>
string MakeRPCCall(const string& server_name, const string& method,
                   const tInput& input, const string& input_cookie,
                   const string& referrer, tOutput* output,
                   string* output_cookie = nullptr, int timeout_ms = -1) {
  // Prepare the request.
  tServerRequestMessage request(method, serial::Serializer::ToBinary(input),
                                input_cookie, referrer);

  return MakeRPCCall(server_name, request, output, output_cookie, timeout_ms);
}

} // namespace network

#endif  // _PUBLIC_UTIL_NETWORK_RPCCLIENT_UTIL_H_
