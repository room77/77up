// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/entity/mock/entity_manager_mock.h"

namespace entity {
namespace test {

void RegisterNewMockEntityManager(const string& id, const string& params) {
  EntityManager::bind(id, "", InitializeConfigureConstructor<MockEntityManager, string>());
}

}  // namespace test
}  // namespace entity
