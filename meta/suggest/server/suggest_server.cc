// Copyright 2013 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)


#include "base/common.h"
#include "util/init/main.h"
#include "util/network/rpcserver.h"

FLAG_int(port, 10013, "Server port number.");

int init_main() {
  network::RPCServer server("suggest_server");
  server.set_portnum(gFlag_port);
  server.PrintProxyConfig();
  server.StartServer();

  return 0;
}
