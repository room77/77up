// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "util/init/main.h"
#include "util/network/rpcserver.h"

FLAG_int(log_server_port, 10016, "Log Server port number.");

int init_main() {
  network::RPCServer server("log_server");

  server.set_portnum(gFlag_log_server_port);
  server.PrintProxyConfig();
  server.StartServer();

  return 0;
}
