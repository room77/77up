//
// Copyright 2007 OpTrip, Inc.
//

#include <chrono>
#include <thread>

#include "util/network/rpcserver.h"
#include "util/network/method/server_method.h"

// the following file contains data structure definitions common to
// both server and client
#include "util/network/tests/rpcstruct_test.h"

// a simple RPC server that performs addition and subtraction of two numbers

FLAG_int(port, 10001, "server port number");

// RPC function definitions

const char sample_server[] = "sample_server";

namespace network {
namespace test {

// function 1: addition
struct tAddition : public ServerMethod {
  string operator()(const tInput& input, int *output) {
    *output = input.a + input.b;
    VLOG(2) << "IP is: " << GetUserIP();
    VLOG(2) << "Country is: " << GetUserCountry();
    VLOG(2) << "calculating: "
           << input.a << " + " << input.b << " -> " << *output;
    return "";  // success
  }

  string Mock(const string& mocktype, const tInput& input, int *output) {
    if (mocktype == "42") {
      VLOG(2) << "mock Add. Return the same thing regardless of input";
      *output = 42;
      return ""; // great success
    }
    return "error mocktype not supported. type: " + mocktype;
  }

 protected:
  virtual bool Validate(ostream& out) {
    bool res = true;
    int output;
    tInput a = {1, 2};
    operator ()(a, &output);
    if (output != 3) {
      out << "Failed: " << a.ToJSON() << " -> " << output << endl;
      res = false;
    }
    return res;
  }
};
ServerMethodRegister<tInput, int, tAddition> reg_tAddition(
    sample_server, "ADD", {1 , 1});

// function 2: subtraction
struct tSubtraction : public ServerMethod {
  string operator()(const tInput& input, int *output) {
    *output = input.a - input.b;
    VLOG(2) << "calculating: "
           << input.a << " - " << input.b << " -> " << *output;
    return "";  // success
  }

 protected:
  virtual bool Validate(ostream& out) {
    bool res = true;
    int output;
    tInput a = {1, 2};
    operator ()(a, &output);
    if (output != -1) {
      out << "Failed: " << a.ToJSON() << " -> " << output << endl;
      res = false;
    }
    return res;
  }
};
ServerMethodRegister<tInput, int, tSubtraction> reg_tSubtraction(
    sample_server, "SUB", {1 , 1});

// function 3: set a cookie
struct tSetCookie : public ServerMethod {
  string operator()(const string& input, bool *output) {
    SetCookie("testcookie", input, 100, "/", "", false, false);
    *output = true;
    return "";  // success
  }

 protected:
  virtual bool Validate(ostream& out) {
    bool res = true;
    bool output;
    operator ()("bad cookie", &output);
    if (1) {  // Force false result.
      out << "Failed: bad cookie" << " -> " << output << endl;
      res = false;
    }
    return res;
  }
};
ServerMethodRegister<string, bool, tSetCookie> reg_tSetCookie(
    sample_server, "SETCOOKIE", "biscuit");

// function 4: retrieve a cookie
struct tGetCookie : public ServerMethod {
  string operator()(bool input, string *output) {
    *output = GetCookie("testcookie");
    return "";  // success
  }
};
ServerMethodRegister<bool, string, tGetCookie> reg_tGetCookie(
    sample_server, "GETCOOKIE", true);

// function 5: Wait for some time.
struct TimedWait : public ServerMethod {
  string operator()(int input, string *output) {
    this_thread::sleep_for(chrono::seconds(5));
    return "";  // success
  }
};
ServerMethodRegister<int, string, TimedWait> reg_TimedWait(
    sample_server, "TimedWait", 1);

}  // namespace test
}  // namespace network

int init_main() {

  ASSERT(gFlag_port > 0) << "port number must be positive!";

  network::RPCServer server("sample_server");
  server.set_portnum(gFlag_port);
  server.PrintProxyConfig();
  server.StartServer();

  return 0;
}
