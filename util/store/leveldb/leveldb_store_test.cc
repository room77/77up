// Copyright 2013 B. Uygar Oztekin

#include <cstdlib>
#include "leveldb_store.h"
#include "../test/test_macros.h"

using namespace std;

string tmpdir = "/tmp/leveldb_test.XXXXXXXX";

void Cleanup() {
  cout << "Cleaning up " << tmpdir << endl;
  // Populate the store with default key / values.
  store::Store<>::mutable_shared_proxy store = store::Store<>::make_shared("leveldb_store");
  cout << "Keys erased   : " << store::test::Tester<store::Store<>>::Unpopulate(*store) << endl;
  assert(system(("rm -rf " + tmpdir).c_str()) == 0);
}

// Register the store.
auto reg_leveldb_store = store::Store<>::bind("leveldb_store", [](){
  return new store::LeveldbStore(tmpdir);
});

struct Setup {
  Setup() {
    assert(mkdtemp(&tmpdir[0]));
    atexit(Cleanup);
    // Populate the store with default key / values.
    store::Store<>::mutable_shared_proxy store = store::Store<>::make_shared("leveldb_store");
    cout << "Keys inserted : " << store::test::Tester<store::Store<>>::Populate(*store) << endl;
  }
} run_now;

// Run the registered tests (see the RULES file).
int main() {
  return store::test::Tester<store::Store<>>::Test("leveldb_store");
}
