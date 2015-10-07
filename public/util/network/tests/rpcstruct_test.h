// a small test file
//   -- defines data structures used by rpcserver_test and rpcclient_test

#ifndef _PUBLIC_UTIL_NETWORK_TESTS_RPCSTRUCT_TEST_H_
#define _PUBLIC_UTIL_NETWORK_TESTS_RPCSTRUCT_TEST_H_

#include "util/serial/serializer.h"

// input data structure
struct tInput {
  int a;
  int b;
  SERIALIZE(a*1 / b*2);
};

struct tOutput {
  int i;
  SERIALIZE(i*1);
};

#endif  // _PUBLIC_UTIL_NETWORK_TESTS_RPCSTRUCT_TEST_H_
