// Copyright 2013 B. Uygar Oztekin

#include <cstdlib>
#include "sst_reader.h"
#include "sst_writer.h"
#include "../test/test_macros.h"
#include "../test/basic.h"
#include "../test/stress.h"
#include "../test/forward_iteration.h"
#include "../test/sorted_forward_iteration.h"
#include "../test/lower_upper_bound.h"

using namespace std;

namespace {

string tmpfile = "/tmp/sst_test.XXXXXXXX";

void Cleanup() {
  cout << "Removing " << tmpfile << endl;
  assert(system(("rm -f " + tmpfile).c_str()) == 0);
}

// Register the store.
auto reg_sst_store = store::Store<>::bind("sst_store", [](){
  return new store::SstReader(tmpfile);
});

struct Setup {
  Setup() {
    assert(mktemp(&tmpfile[0]));
    atexit(Cleanup);
    // Populate the store with default key / values.
    store::SstWriter writer(tmpfile);
    cout << "Keys inserted : " << store::test::Tester<store::Store<>>::Populate(writer) << endl;
  }
} run_now;

}

// Run the registered tests (see the RULES file).
int main() {
  return store::test::Tester<store::Store<>>::Test("sst_store");
}
