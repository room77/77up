// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// @include "entity_parser_em.cc"

#include "util/entity/parser/entity_parser.h"

#include "test/cc/test_main.h"
#include "util/entity/mock/entity_mock.h"
#include "util/entity/mock/entity_manager_mock.h"

namespace entity {
namespace parser {
namespace test {

using ::entity::test::MockEntityManager;
using ::entity::test::RegisterNewMockEntityManager;
using ::entity::test::GetMockEntity;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;

// Test class for MyTest.
class EntityParserEMTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    RegisterNewMockEntityManager("mock_em");
  }

  static void TearDownTestCase() {}

  static const string MockParams() {
    static const string kParams = "{\"em_id\": \"mock_em\"}";
    return kParams;
  }

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    SetUpDefaults();
  }

  // Sets up default values for all values.
  void SetUpDefaults() {
    parser_ = EntityParser::make_shared("entity_parser_em", MockParams());
    ASSERT_NOTNULL(parser_);

    mock_em_proxy_ = EntityManager::make_shared("mock_em");
    mock_em_ = dynamic_cast<MockEntityManager*>(mock_em_proxy_.get());
    ASSERT_NOTNULL(mock_em_);
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  EntityParser::shared_proxy parser_;
  EntityManager::mutable_shared_proxy mock_em_proxy_;
  MockEntityManager* mock_em_ = nullptr;
};

TEST_F(EntityParserEMTest, Setup) {
}

TEST_F(EntityParserEMTest, EmptyParams) {
  parser_ = EntityParser::make_shared("entity_parser_em", "");
  EXPECT_EQ(nullptr, parser_);
}

TEST_F(EntityParserEMTest, InvalidParams) {
  string params = "{\"em_id\": \"invalid_em\"}";

  parser_ = EntityParser::make_shared("entity_parser_em", params);
  EXPECT_EQ(nullptr, parser_);
}

TEST_F(EntityParserEMTest, EmptyQuery) {
  EXPECT_EQ(nullptr, parser_->Parse(""));
}

TEST_F(EntityParserEMTest, Sanity) {
  string eid = "c/ABC";
  tEntity entity = GetMockEntity(eid);
  EXPECT_CALL(*mock_em_, LookupUniqueByEntityId(eid)).WillOnce(Return(&entity));

  const tEntity* reply = parser_->Parse(eid);
  ASSERT_NOTNULL(reply);
  EXPECT_EQ(eid, reply->GetEid());
}

TEST_F(EntityParserEMTest, ManagerReturnsNullptr) {
  string eid = "s/ABC";
  tEntity entity = GetMockEntity(eid);
  EXPECT_CALL(*mock_em_, LookupUniqueByEntityId(eid)).WillOnce(Return(nullptr));

  const tEntity* reply = parser_->Parse(eid);
  EXPECT_EQ(nullptr, reply);
}

}  // namespace test
}  // namespace parser
}  // namespace entity
