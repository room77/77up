// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/network/method/server_method.h"

namespace network {
namespace {

class Ping : public network::ServerMethod {
 public:
  string operator()(const int req, int* reply) const {
    *reply = 1;
    return "";
  }
};
ServerMethodRegister<int, int, Ping> reg_Ping("all", ".ping", 0);

}  // namespace
}  // namespace network
