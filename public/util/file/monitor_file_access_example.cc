// Copyright 2014 Room77, Inc.
// Author: B. Uygar Oztekin

#include <fstream>

#include "monitor_file_access.h"
#include "util/init/main.h"

void TouchFile(const string& filename) {
  ifstream file(filename.c_str());
}

extern string gFlag_record_file_access_output;

int init_main() {
  gFlag_record_file_access = true;
  gFlag_record_file_access_output = "/dev/stdout";
  TouchFile("/dev/null");
  TouchFile("/etc/hosts");
  TouchFile("/etc/hostname");
  TouchFile("/does/not/exist/skip");
  return 0;
}
