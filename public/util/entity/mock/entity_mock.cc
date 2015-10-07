// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/entity/mock/entity_mock.h"

namespace entity {
namespace test {

tMockEntity GetMockEntity(const string& eid, EntityType type) {
  tMockEntity entity(type);
  entity.SetEid(eid);
  return entity;
}

}  // namespace test
}  // namespace entity
