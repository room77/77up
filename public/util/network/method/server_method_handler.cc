// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/network/method/server_method_handler.h"

FLAG_int(max_binary_response_size, 8388608,
         "Max. size a binary serialized response can have.");

FLAG_int(max_json_response_size, 8388608,
         "Max. size a JSON serialized response can have.");


using namespace network;

void ServerMethodHandler2::ProcessRPC(const tConnectionInfo* connection,
                                      const tServerRequestMessage& reqMsg,
                                      tServerReplyMessage* response) const {
  response->Clear();
  VLOG(3) << "processrpc:";
  // strutil::PrintRaw(reqMsg.message);
  // parse input from serialized string
  unique_ptr<tMessageBase> req(ServerMethod->CreateRequest());
  if (!req->FromBinary(reqMsg.message)) {
    // error during input parsing
    response->message = "RPC parsing error -- server mismatch?";
    response->success = false;
    return;
  }

  string error_msg;

  // fill request
  error_msg = ServerMethod->FillRequest(*connection, reqMsg, req.get());
  if (!error_msg.empty()) {
    response->message = error_msg;
    response->success = false;
    return;
  }

  // Run the server method.
  unique_ptr<tMessageBase> res(ServerMethod->CreateResponse());
  error_msg = (*ServerMethod)(*req, res.get(), &response->cookies);
  if (!error_msg.empty()) {
    response->message = error_msg;
    response->success = false;
    return;
  }
  // return output in serialized string format
  response->message = res->ToBinary();
  ASSERT_LE(response->message.size(), gFlag_max_binary_response_size)
    << "Serialized Binary response too large (size="
    << response->message.size()
    << ").  Please increase command-line flag max_binary_response_size.";

  response->success = true;
}

void ServerMethodHandler2::ProcessJSON(const tConnectionInfo* connection,
                                       const tServerRequestMessage& reqMsg,
                                       bool debug_json,
                                       tServerReplyMessage* response) const {
  response->Clear();
  VLOG(4) << "processjson: " << reqMsg.message;
  // parse input from serialized string
  string error_msg;
  unique_ptr<tMessageBase> req(ServerMethod->CreateRequest());
  if (!req->FromJSON(reqMsg.message, &error_msg)) {
    // error: unable to parse JSON input
    tErrorMessage_JSON err;
    err.error_msg = error_msg;
    response->message = err.ToJSON({2});
    response->success = false;
    return;
  }
  error_msg = ServerMethod->FillRequest(*connection, reqMsg, req.get());
  if (!error_msg.empty()) {
    tErrorMessage_JSON err;
    err.error_msg = error_msg;
    response->message = err.ToJSON({2});
    response->success = false;
  }


  // Run the server method.
  unique_ptr<tMessageBase> res(ServerMethod->CreateResponse());
  error_msg = (*ServerMethod)(*req, res.get(), &response->cookies);
  if (!error_msg.empty()) {
    // error: return error message to caller
    tErrorMessage_JSON err;
    err.error_msg = error_msg;
    response->message = err.ToJSON({2});
    response->success = false;
  }

  // return output in JSON format
  // (if debug_json is true, format JSON output to be human-readable)
  response->message = res->ToJSON(debug_json ? 2 : 0);
  ASSERT_LE(response->message.size(), gFlag_max_json_response_size)
    << "Serialized JSON response too large (size="
    << response->message.size()
    << ").  Please increase command-line flag max_json_response_size.";
  response->success = true;
}

string ServerMethodHandler2::RPCToJSON(const string& message, int indent) const {
  unique_ptr<tMessageBase> req(ServerMethod->CreateRequest());
  if (req->FromBinary(message)) {
      return req->ToJSON(indent);
  } else {
    return "[Error in RPC string]";
  }
}
string ServerMethodHandler2::GetSampleInput_JSON() const {
  return ServerMethod->ExampleJSON();
}

ServerMethodStatus ServerMethodHandler2::Validate(const tConnectionInfo* connection,
    const tServerRequestMessage& reqMsg, ostream& out) const {
  unique_ptr<tMessageBase> req(ServerMethod->CreateRequest());
  string error_msg = ServerMethod->FillRequest(*connection, reqMsg, req.get());
  if (!error_msg.empty()) {
    out << error_msg << endl;
    return kServerMethodStatusInvalid;
  }
  return ServerMethod->Validate(*req, out);
}
