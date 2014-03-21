// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/entity/parser/mock/entity_parser_mock.h"

#include "util/factory/factory_extra.h"

namespace entity {
namespace parser {
namespace test {

void RegisterNewMockEntityParser(const string& id, const string& params) {
  EntityParser::bind(id, "", InitializeConfigureConstructor<MockEntityParser, string>());
}

}  // namespace test
}  // namespace parser
}  // namespace entity
