//
// Copyright 2007 OpTrip, Inc.
//

#include "util/init/main.h"
#include "util/network/httpclient.h"

int init_main() {

  int status_code;
  string reply;

  HttpClient h;
  ASSERT(h.HttpGet("www.google.com", 80, "/", &status_code, &reply))
    << "Error during GET";
  ASSERT_EQ(status_code, 200);

  LOG(INFO) << "Status code is: " << status_code << "\n";
  LOG(INFO) << "Reply is:\n" << reply << "\n";

  return 0;
}

