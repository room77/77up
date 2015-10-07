// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/entity/parser/group/entity_parser_group.h"

#include "test/cc/test_main.h"
#include "util/entity/entity_id.h"
#include "util/entity/mock/entity_mock.h"
#include "util/entity/parser/mock/entity_parser_mock.h"
#include "util/factory/factory_extra.h"

namespace entity {
namespace parser {
namespace test {

using ::entity::test::GetMockEntity;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;

// Test class for MyTest.
class EntityParserGroupTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    EntityParser::bind("entity_parser_group_test", "",
        InitializeConfigureConstructor<EntityParserGroup, string>());

    RegisterNewMockEntityParser("mock_parser1");
    RegisterNewMockEntityParser("mock_parser2");
  }

  static void TearDownTestCase() {}

  static string MockParams() {
    static const string kParams =
        "{"
          "\"parser_params\": ["
            "{"
              "\"entity_type\": " + to_string(entity::kEntityTypeCity) + ","
              "\"parser_id\": \"mock_parser1\","
            "}, {"
              "\"entity_type\": " + to_string(entity::kEntityTypeState) + ","
              "\"parser_id\": \"mock_parser2\","
            "}"
          "]"
        "}";

    return kParams;
  }

 protected:
  // Sets up the test fixture.
  virtual void SetUp() {
    SetUpDefaults();
  }

  // Sets up default values for all values.
  void SetUpDefaults() {
    group_parser_ = EntityParser::make_shared("entity_parser_group_test", MockParams());
    ASSERT_NOTNULL(group_parser_);

    mock_parser_proxy_1_ = EntityParser::make_shared("mock_parser1");
    mock_parser_proxy_2_ = EntityParser::make_shared("mock_parser2");
    mock_parser1_ = dynamic_cast<MockEntityParser*>(mock_parser_proxy_1_.get());
    mock_parser2_ = dynamic_cast<MockEntityParser*>(mock_parser_proxy_2_.get());
    ASSERT_NOTNULL(mock_parser1_);
    ASSERT_NOTNULL(mock_parser2_);
  }

  // Tears down the test fixture.
  virtual void TearDown() {}

  EntityParser::shared_proxy group_parser_;
  EntityParser::mutable_shared_proxy mock_parser_proxy_1_;
  EntityParser::mutable_shared_proxy mock_parser_proxy_2_;
  MockEntityParser* mock_parser1_ = nullptr;
  MockEntityParser* mock_parser2_ = nullptr;
};

TEST_F(EntityParserGroupTest, Setup) {
}

TEST_F(EntityParserGroupTest, EmptyParams) {
  group_parser_ = EntityParser::make_shared("entity_parser_group_test", "");
  EXPECT_EQ(nullptr, group_parser_);
}

TEST_F(EntityParserGroupTest, InvalidParams) {
  string params = "{"
      "{"
        "\"parser_params\": ["
          "{"
            "\"entity_type\": " + to_string(entity::kEntityTypeCity) + ","
            "\"parser_id\": \"invalid_parser\","
          "}, {"
            "\"entity_type\": " + to_string(entity::kEntityTypeState) + ","
            "\"parser_id\": \"mock_parser2\","
          "}"
        "]"
      "}";

  group_parser_ = EntityParser::make_shared("entity_em_parser", params);
  EXPECT_EQ(nullptr, group_parser_);
}

TEST_F(EntityParserGroupTest, EmptyQuery) {
  EXPECT_EQ(nullptr, group_parser_->Parse(""));
}

TEST_F(EntityParserGroupTest, Sanity) {
  string eid = "c/ABC";
  tEntity entity = GetMockEntity(eid);
  EXPECT_CALL(*mock_parser1_, Parse(eid)).WillOnce(Return(&entity));
  // Mock 2 should not be called.
  EXPECT_CALL(*mock_parser2_, Parse(_)).Times(0);

  const tEntity* reply = group_parser_->Parse(eid);
  ASSERT_NOTNULL(reply);
  EXPECT_EQ(eid, reply->GetEid());
}

TEST_F(EntityParserGroupTest, Sanity2) {
  string eid = "s/ABC";
  tEntity entity = GetMockEntity(eid);
  EXPECT_CALL(*mock_parser2_, Parse(eid)).WillOnce(Return(&entity));
  // Mock 1 should not be called.
  EXPECT_CALL(*mock_parser1_, Parse(_)).Times(0);

  const tEntity* reply = group_parser_->Parse(eid);
  ASSERT_NOTNULL(reply);
  EXPECT_EQ(eid, reply->GetEid());
}

TEST_F(EntityParserGroupTest, NoManagerForPrefix) {
  string eid = "y/ABC";

  // None of the registered mocks should be called.
  EXPECT_CALL(*mock_parser1_, Parse(_)).Times(0);
  EXPECT_CALL(*mock_parser2_, Parse(_)).Times(0);

  const tEntity* reply = group_parser_->Parse(eid);
  EXPECT_EQ(nullptr, reply);
}

TEST_F(EntityParserGroupTest, ManagerReturnsNullptr) {
  string eid = "s/ABC";
  tEntity entity = GetMockEntity(eid);
  EXPECT_CALL(*mock_parser2_, Parse(eid)).WillOnce(Return(nullptr));
  // Mock 1 should not be called.
  EXPECT_CALL(*mock_parser1_, Parse(_)).Times(0);

  const tEntity* reply = group_parser_->Parse(eid);
  EXPECT_EQ(nullptr, reply);
}

}  // namespace test
}  // namespace parser
}  // namespace entity
