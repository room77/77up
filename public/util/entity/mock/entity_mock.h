// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_ENTITY_MOCK_ENTITY_MOCK_H_
#define _UTIL_ENTITY_MOCK_ENTITY_MOCK_H_

#include "util/entity/entity.h"
#include "test/cc/test_main.h"

namespace entity {
namespace test {

struct tMockEntity : public tEntity {
  explicit tMockEntity(EntityType type = kEntityTypeCity) : type(type) {}

  virtual EntityType GetEntityType() const { return type; }

  EntityType type;
};

tMockEntity GetMockEntity(const string& eid = "c/sf", EntityType type = kEntityTypeCity);

}  // namespace test
}  // namespace entity


#endif  // _UTIL_ENTITY_MOCK_ENTITY_MOCK_H_
