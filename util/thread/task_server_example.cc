// Copyright 2012 Room77, Inc.
// Author: Uygar Oztekin

// This test is meant to be linked against util/task/task_server.

#include <thread>
#include <chrono>

#include "base/common.h"
#include "util/network/rpcserver.h"
#include "util/thread/task_server.h"

FLAG_int(port, 10000, "Server port number.");

namespace {
bool Test() {
  LOG(INFO) << "Test called. Will sleep for 2 seconds.";
  this_thread::sleep_for(chrono::seconds(2));
  LOG(INFO) << "Woke up";
  return true;
}
auto register_test_task = task::Task<>::bind("test", []{return new task::Task<>(&Test);});
auto register_test_alias = task::Task<>::alias("test1", "test");
auto register_test_task2 = task::Task<>::bind("test2", []{return new task::Task<>(&Test);});
}

int init_main() {

  network::RPCServer server;
  server.RegisterHandler<task::Start::Request, task::Start::Reply, task::Start>(
      "TaskStart", task::Start::Request{"test"});
  server.RegisterHandler<task::Status::Request, task::Status::Reply, task::Status>(
      "TaskStatus", task::Status::Request{"test"});
  server.RegisterHandler<task::History::Request, task::History::Reply, task::History>(
      "TaskHistory", task::History::Request{});

  server.set_portnum(gFlag_port);
  server.PrintProxyConfig();
  server.StartServer();

  return 0;
}
