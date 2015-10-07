// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_ENTITY_MOCK_ENTITY_MANAGER_MOCK_H_
#define _UTIL_ENTITY_MOCK_ENTITY_MANAGER_MOCK_H_

#include "util/entity/entity_manager.h"

#include "test/cc/test_main.h"

namespace entity {
namespace test {

class MockEntityManager : public EntityManager {
 public:
  // By default return true.
  virtual bool Configure(const string& opts) { return true; }
  virtual bool Initialize() { return true; }

  MOCK_CONST_METHOD1(LookupUniqueByEntityId,
      const tEntity*(const string& entity_id));
  MOCK_CONST_METHOD1(LookupMutableByEntityId,
      tEntity*(const string& entity_id));
};

void RegisterNewMockEntityManager(const string& id, const string& params = "");

}  // namespace test
}  // namespace entity


#endif  // _UTIL_ENTITY_MOCK_ENTITY_MANAGER_MOCK_H_
