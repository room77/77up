// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Mock file for Query to Eid converter.

#ifndef _UTIL_ENTITY_PARSER_MOCK_ENTITY_PARSER_MOCK_H_
#define _UTIL_ENTITY_PARSER_MOCK_ENTITY_PARSER_MOCK_H_

#include "util/entity/parser/entity_parser.h"

#include "test/cc/test_main.h"

namespace entity {
namespace parser {
namespace test {

class MockEntityParser : public EntityParser {
 public:
  // By default return true.
  virtual bool Configure(const string& opts) { return true; }
  virtual bool Initialize() { return true; }

  MOCK_CONST_METHOD1(Parse, const tEntity*(const string& eid));
};

// Register a new mock entity parser for the given id with the given params.
void RegisterNewMockEntityParser(const string& id, const string& params = "");

// Get a new mock entity parser for the given id with the given params.
MockEntityParser* MakeNewMockEntityParser(const string& id, const string& params = "");

}  // namespace test
}  // namespace parser
}  // namespace entity

#endif  // _UTIL_ENTITY_PARSER_MOCK_ENTITY_PARSER_MOCK_H_
