// Copyright 2013 Room77, Inc.
// Author: Uygar Oztekin

// Basic tester for std::string to uint{32,64,128}_t hashers.
// Automatically scans and tests all registered hashers.

#include <cassert>
#include <bitset>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include "util/hash/hasher.h"

namespace hash {

// Generate strings corresponding to numbers from start to limit.
const vector<string>& NumericData() {
  constexpr int start = 0;
  constexpr int limit = 1 << 22;
  struct Init {
    vector<string> operator()() {
      vector<string> ret;
      ret.reserve(limit - start);
      for (int i = start; i < limit; ++i) {
        stringstream ss;
        ss << i;
        ret.push_back(ss.str());
      }
      return ret;
    }
  };
  static vector<string> data = Init()();
  return data;
}

uint64_t Now() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

template<class Input, class Output>
int TestHasher(const string& id, const vector<string>& container) {
  constexpr uint64_t mod = 1UL << 32;
  vector<bool> collision_map(mod, false);
  auto hasher = Hasher<Input, Output>::make_shared(id);
  assert(hasher.get());

  int collisions = 0;
  auto t0 = Now();
  const auto& keys = NumericData();
  bool success = true;
  cout << "  " << id << endl;
  for (int i = 0; i < NumericData().size(); ++i) {
    auto hash = hasher->operator()(keys[i]);
    uint32_t bucket = hash % mod;
    if (collision_map[bucket]) ++collisions;
    collision_map[bucket] = true;
    if (i > (1 << 12) && !((i - 1) & i)) {
      cout << "    n:" << setw(8) << i
           << ", col:" << setw(5) << collisions
           << ", time (ms): " << (Now() - t0) / 1000 << endl;
      // We do not tolerate more than a few collisions for 2^16 entries.
      // Ideally, it should still be zero for 2^16 items for good hashers.
      // Note that we use an effective hash space of 32 bits due to modula op.
      if (i <= (1 << 16) && collisions > 2) success = false;
    }
  }
  return success;
}

template<class Input, class Output>
bool TestHashers() {
  bool success = true;
  vector<string> keys;
  Hasher<Input, Output>::append_keys(keys);
  cout << " ==== " << setw(3) << sizeof(Output) * 8 << " bits hash modula 2^32 ====" << endl;
  for (auto& key : keys) {
    success &= TestHasher<Input, Output>(key, NumericData());
    cout << endl;
  }
  return success;
}

}

int main() {
  bool success = true;
  success &= ::hash::TestHashers<string, uint32_t>();
  success &= ::hash::TestHashers<string, uint64_t>();
  success &= ::hash::TestHashers<string, uint128_t>();
  return success ? 0 : -1;
}
