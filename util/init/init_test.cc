// Copyright 2013 Room77, Inc.
// Author: B. Uygar Oztekin

#include <iostream>
#include <cassert>
#include <thread>
#include <string>
#include "init.h"
#include "main.h"

using namespace std;

int state[] = {0,0,0,0,0,0,0,0,0,0};

string gFlag_test_flag = "default value";

namespace test {
const string& UberComplexStuff() {
  struct Init {
    const string& operator()() {
      this_thread::sleep_for(chrono::milliseconds(300));
      cout << "UberComplexStuff initialized" << endl;
      return gFlag_test_flag;
    }
  };
  static string data = Init()();
  return data;
}
};

string WaitSome() {
  this_thread::sleep_for(chrono::milliseconds(rand()%300));
  return "";
}

int Flip(int i) {
  state[i] = 1;
  return i;
}

INIT_ADD("", 1, []{ cout << Flip(0) << " should run last." << endl; });
INIT_ADD("",    []{ cout << Flip(1) << WaitSome() << endl; });
INIT_ADD("",    []{ cout << Flip(2) << WaitSome() << endl; });
INIT_ADD("g1",  []{ cout << Flip(3) << WaitSome() << endl; });
INIT_ADD("g1",  []{ cout << Flip(4) << WaitSome() << endl; });
INIT_ADD("g1",  []{ cout << Flip(5) << WaitSome() << endl; });
INIT_ADD("g2",  []{ cout << Flip(6) << WaitSome() << endl; });
INIT_ADD("g2",  []{ cout << Flip(7) << WaitSome() << endl; });
INIT_ADD("g2", -1, []{
  cout << Flip(8)
       << " should run first. Value I have: "
       << test::UberComplexStuff() << endl;
});
INIT_ADD_REQUIRED("req", []{ cout << "Req init " << Flip(9) << WaitSome() << endl; });

EXIT_ADD("",   []{ cout << "Exit func " << 0 << WaitSome() << endl; });
EXIT_ADD("",   []{ cout << "Exit func " << 1 << WaitSome() << endl; });
EXIT_ADD("g1", []{ cout << "Exit func " << 2 << WaitSome() << endl; });
EXIT_ADD("g1", []{ cout << "Exit func " << 3 << WaitSome() << endl; });
EXIT_ADD("g2", []{ cout << "Exit func " << 4 << WaitSome() << endl; });
EXIT_ADD("g2", []{ cout << "Exit func " << 5 << WaitSome() << endl; });
EXIT_ADD("g3", []{ cout << "Exit func " << 6 << WaitSome() << endl; });
EXIT_ADD("g3", []{ cout << "Exit func " << 7 << WaitSome() << endl; });
EXIT_ADD_REQUIRED("req", []{ cout << "Req exit" << WaitSome() << endl; });

// Required versions. If you change --{init,exit}_{indclude,exclude} these
// should always run.


// init_main() automatically handles all initializations.
// No need to pass argc, argv anymore.
int init_main() {
  // Make sure all initializations are done before we get in init_main().
  float mult = 1;
  for (int i = 0; i < sizeof(state)/sizeof(int); ++i) {
    cout << "state " << i << ": " << state[i] << endl;
    mult *= state[i];
  }
  assert(mult == 1);
  assert(test::UberComplexStuff() == "default value");
  return 0;
}
