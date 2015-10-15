// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)
// File created as part of refactoring rpcserver.h

// Class for forwarding calls to remote server method.

#ifndef _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_FORWARD_H_
#define _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_FORWARD_H_

#include "base/common.h"
#include "util/network/rpcclient_util.h"
#include "util/network/method/server_method.h"
#include "util/network/method/server_method_forward_collection.h"
#include "util/network/method/server_method_handler.h"

namespace network {

// Template to forward an RPC call to another server.
// The default timeout is 10 seconds.
template<class tInput, class tOutput, const char* server_type,
    const char* remote_method, int timeout = 10>
class ServerMethodForward : public ServerMethod {
 public:
  string operator() (const tInput& req, tOutput* result) {
    static_assert(server_type != nullptr, "server_type cannot be null");
    static_assert(remote_method != nullptr, "server_type cannot be null");
    // forward the request to remote server
    string output_cookie;
    string error_msg = MakeRPCCall(server_type, remote_method,
                                   req, input_cookie(), referrer(),
                                   result, &output_cookie, timeout * 1000);
    add_output_cookie(output_cookie);  // pass the output cookie back
    return error_msg;
  }
};

// Structure to register a new forwarded server method with the server.
template<class tInput, class tOutput, const char* server_type,
    const char* remote_method, int timeout = 10>
struct ServerMethodForwardRegister {
  ServerMethodForwardRegister(const string& server_name,
                              const string& local_method,
                              const tInput& sample_input) {
    typedef ServerMethodForward<tInput, tOutput, server_type, remote_method,
        timeout> SMForward;
    ServerMethodRegister<tInput, tOutput, SMForward> reg(
        server_name, local_method, sample_input);

    // We want to get a static proxy as we want an instance to stay around for
    // the lifetime of the binary.
    ServerMethodForwardCollection::shared_proxy proxy =
        ServerMethodForwardCollection::GetCollectionForServer(server_name);
    proxy->Register(local_method, {server_type, remote_method, timeout});
    // Pin this proxy to ensure it is never destroyed.
    ServerMethodForwardCollection::pin(proxy);
  }
};

}  // namespace network

#endif  // _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_FORWARD_H_
