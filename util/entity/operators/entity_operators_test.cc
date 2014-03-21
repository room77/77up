// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/entity/operators/entity_operators.h"

#include "test/cc/test_main.h"
#include "util/entity/entity_manager.h"

namespace entity {
namespace op {
namespace test {

// Test class for MyTest.
class OperatorsManagerTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    manager_ = EntityManager::make_shared("em_operators");
    ASSERT_NOTNULL(manager_);
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  EntityManager::shared_proxy manager_;

};

TEST_F(OperatorsManagerTest, Proxy) {
}

TEST_F(OperatorsManagerTest, Sanity) {
  const tEntity* entity = manager_->LookupUniqueByEntityId("o/+");
  EXPECT_EQ(kEntityTypeOperator, entity->GetEntityType());
  const tOperator* op = dynamic_cast<const tOperator*>(entity);
  ASSERT_NOTNULL(op);
  EXPECT_EQ("+", op->combiner);
}

TEST_F(OperatorsManagerTest, TestBasicOps) {
  vector<string> op_ids = {"+", "&", "~"};
  for (const string& op_id : op_ids) {
    const tEntity* entity = manager_->LookupUniqueByEntityId("o/" + op_id);
    EXPECT_EQ(kEntityTypeOperator, entity->GetEntityType());
    EXPECT_EQ("o/" + op_id, entity->eid);
    const tOperator* op = dynamic_cast<const tOperator*>(entity);
    ASSERT_NOTNULL(op);
    EXPECT_EQ(op_id, op->combiner);
  }
}

TEST_F(OperatorsManagerTest, InvalidQuery) {
  EXPECT_EQ(nullptr, manager_->LookupUniqueByEntityId("o/-"));
}

}  // namespace test
}  // namespace op
}  // namespace entity
