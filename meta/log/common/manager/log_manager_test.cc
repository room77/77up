// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "meta/log/common/manager/log_manager.h"

#include "test/cc/test_main.h"

namespace logging {
namespace test {

TEST(ValidateElementData, Sanity) {
  shared_ptr<tLogElement> element(new tLogElement);
  element->id = "1bc22H";
  element->pid = "";
  element->nid = "AAAsss";
  uint8_t element_valid = event::ValidatorBase::kElementDataBit;
  EXPECT_EQ(element_valid, LogManager::Instance().ValidateElementData(element));
}

// TODO(otasevic): re-enable these two tests once the id alphanumeric 6-character check is enabled
/*TEST(ValidateElementData, NonAlphaNumeric) {
  shared_ptr<tLogElement> element(new tLogElement);
  element->id = "1@2222";
  EXPECT_EQ(0, LogManager::Instance().ValidateElementData(element));
}

TEST(ValidateElementData, NonExpectedSize) {
  shared_ptr<tLogElement> element(new tLogElement);
  element->id = "sadsadsadsa";
  EXPECT_EQ(0, LogManager::Instance().ValidateElementData(element));
}*/

}  // namespace test
}  // namespace logging
