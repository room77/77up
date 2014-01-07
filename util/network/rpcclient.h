//
// Copyright 2007 OpTrip, Inc.
//

#ifndef _PUBLIC_UTIL_NETWORK_RPCCLIENT_H_
#define _PUBLIC_UTIL_NETWORK_RPCCLIENT_H_

#include "util/network/rpc_datatypes.h"
#include "util/network/netclient.h"
#include "util/codetranslator/server_config.h"
#include "util/serial/encoding/encoding.h"
#include "util/serial/serializer.h"

// An RPC client cannot be reused for multiple calls in parallel.
class RPCClient : public NetClient {
 public:
  virtual ~RPCClient() {};

  template<class tOutput>
    static string ParseRPCResponse(const string& reply,
                                   tOutput* output,
                                   string* output_cookie_full_spec) {
    // reply is a of tMessageWithCookie format -- decode it first
    tServerReplyMessage reply_msg;
    if (reply.empty() ||
        !serial::Serializer::FromBinaryPrependedSize(reply, &reply_msg))
      return "Server error -- Server does not support RPC, "
          "or invalid server response.";

    if (reply_msg.success) {
      if (reply_msg.message.empty())
        return "Server error -- RPC reply is empty";

      // reply indicates successful RPC call

      // further deserialize it into appropriate data structure
      if (!serial::Serializer::FromBinary(reply_msg.message, output))
        return "Server error -- Could not parse server message";

      // TODO(edelman) - this only accepts one cookie being set. need to accept multiple
      if (output_cookie_full_spec && !reply_msg.cookies.empty())
        *output_cookie_full_spec = reply_msg.cookies[0];
      return "";  // success
    } else {
      // server reported an error
      if (reply_msg.message.empty())
        return "Server error -- empty error message detected";
      else return reply_msg.message;
    }
  }

  // make an RPC call (with tServerRequestMessage)
  template<class tOutput>
  string Call(const tServerRequestMessage& request,
              tOutput* output, string* output_cookie_full_spec) {
    if (output_cookie_full_spec)
      output_cookie_full_spec->clear();

    // encode the request with size prepended.
    string msg = serial::Serializer::ToBinaryPrependSize(request);
    VLOG(4) << "Request: size = " <<  msg.size() << ",\n" <<
        serial::encoding::EscapeString(msg);

    // send message to server.
    if (!SendMessage(msg))
      return "Error while sending RPC message to server";

    // wait for server to respond
    if (!WaitForReply())
      return "Server timeout or error";

    return ParseRPCResponse(reply_, output, output_cookie_full_spec);
  }

  // make an RPC call (with cookie, referrer, etc.)
  template<class tInput, class tOutput>
  string Call(const string& method, const tInput& input,
              const string& input_cookie, const string& referrer,
              tOutput* output, string* output_cookie_full_spec) {
    // Prepare the request.
    tServerRequestMessage request(method, serial::Serializer::ToBinary(input),
                                  input_cookie, referrer);

    return Call(request, output, output_cookie_full_spec);
  }

  // make a simple RPC call (without cookie, referrer, etc.)
  template<class tInput, class tOutput>
  inline string CallWithoutCookie(const string& method,
                                  const tInput& input, tOutput* output) {
    // Prepare the request.
    tServerRequestMessage request(method, serial::Serializer::ToBinary(input));
    return Call(request, output, nullptr);
  }

 private:
  // incoming reply data is complete if it contains a complete string
  virtual bool ReplyIsComplete() const {
    return RPCMessageIsComplete(reply_);
  }
};

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
    err = "Cannot establish connection to " + server_name + "::" + request.opname;
    return err;
  }

  err = client.Call(request, output, output_cookie);

  if (!err.empty()) {
    LOG(INFO) << "Error msg from cache_server request " << request.opname
           << ": " << err;
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


#endif  // _PUBLIC_UTIL_NETWORK_RPCCLIENT_H_
