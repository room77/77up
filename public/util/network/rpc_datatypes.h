// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The data types for RPC client server data transfer.

#ifndef _PUBLIC_UTIL_NETWORK_RPC_DATATYPES_H_
#define _PUBLIC_UTIL_NETWORK_RPC_DATATYPES_H_

#include "base/common.h"
#include "util/serial/serializer.h"

// RPC request format
struct tServerRequestMessage {
  tServerRequestMessage(const string& opname = "", const string& m = "",
                        const string& c = "", const string& r = "",
                        const unordered_map<string, string>& http_header = unordered_map<string, string>{},
                        const unordered_map<string, string>& arg_map = unordered_map<string, string>{})
   : opname(opname), message(m), referrer(r), cookie(c),
     http_header(http_header), arg_map(arg_map) {}

  void Clear() {
    opname.clear();
    message.clear();
    cookie.clear();
    referrer.clear();
    http_header.clear();
    arg_map.clear();
  }

  void MergeMetaData(const tServerRequestMessage& right) {
    // Only overwrite referrer if we don't have one already.
    if (referrer.empty()) referrer = right.referrer;

    // We prepend it as whatever comes latere overwrites earlier seen cookies.
    if (!right.cookie.empty()) cookie.insert(0, right.cookie);

    // If the value already exists, it is not overwritten.
    for (const auto& p : right.http_header) http_header.insert(p);

    // If the value already exists, it is not overwritten.
    for (const auto& p : right.arg_map) arg_map.insert(p);
  }

  // Th name of the method to execute.
  string opname;
  // The request for the method in binary format.
  string message;
  string referrer;
  // The input cookie
  string cookie;

  // These fields are only used in '_rpc'.
  unordered_map<string, string> http_header;
  unordered_map<string, string> arg_map;

  SERIALIZE(opname*1 / message*2 / referrer*3 /cookie*4 / http_header*5 /
            arg_map*6);
};

// RPC reply format.
struct tServerReplyMessage {
  void Clear() {
    message.clear();
    cookies.clear();
    success = false;
  }

  bool success = false;
  // The response from the message (in binary format).
  string message;
  // The output cookies strings.
  vector<string> cookies;

  SERIALIZE(success*1 / message*2 / cookies*3);
};

// Returns true if the RPC message has been completely received.
inline bool RPCMessageIsComplete(const string& str) {
  return serial::Serializer::BinaryStreamHasEnoughDataToParsePrependedSize<unsigned int>(str);
}

#endif  // _PUBLIC_UTIL_NETWORK_RPC_DATATYPES_H_
