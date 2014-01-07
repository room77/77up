// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: yangc@room77.com (Calvin Yang), pramodg@room77.com (Pramod Gupta)

#include <iostream>
#include <cassert>
#include "args.h"

FLAG_int(flag1, 0, "a test variable");
FLAG_string(flag2, "hello", "another test variable");
FLAG_bool(flag3, false, "another test variable");

int main() {
  // this unittest only -- check default values
  assert(gFlag_flag1 == 0);
  assert(gFlag_flag2 == "hello");
  assert(gFlag_flag3 == false);

  cout << "value of flag1: " << gFlag_flag1 << endl;
  cout << "value of flag2: " << gFlag_flag2 << endl;
  cout << "value of flag3: " << gFlag_flag3 << endl;

  return 0;
}

