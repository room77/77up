// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

#include <thread>
#include "util/init/main.h"
#include "util/network/cached_httpclient.h"

void LaunchGet(int n) {
  CachedHttpClient<> h;
  int status_code;
  string reply;
  bool success = h.HttpGet("www.google.com", 80, "/", &status_code, &reply);
  cout << "Thread: " << n << " " << success << " " << status_code << " " << reply.size() << endl;
}

void LaunchPost(int n) {
  CachedHttpClient<> h;
  int status_code;
  string reply;
  bool success = h.HttpPost("www.google.com", 80, "/", "", &status_code, &reply);
  cout << "Thread: " << n << " " << success << " " << status_code << " " << reply.size() << endl;
}

int init_main() {

  vector<thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.push_back(thread(bind(LaunchGet, i)));
  }
  for (int i = 10; i < 20; ++i) {
    threads.push_back(thread(bind(LaunchPost, i)));
  }

  for (int i = 0; i < threads.size(); ++i) {
    threads[i].join();
  }

  CachedHttpClient<>::DumpStats();

  return 0;
}

