// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// The different operators that can be present in the entity id string.

#ifndef _UTIL_ENTITY_OPERATORS_ENTITY_OPERATORS_H_
#define _UTIL_ENTITY_OPERATORS_ENTITY_OPERATORS_H_

#include "base/defs.h"

#include "util/entity/entity.h"
#include "util/serial/serializer.h"

namespace entity {
namespace op {

// The struct that defines each operator.
struct tOperator : public tEntity {
  virtual ~tOperator() {}

  // Returns the entity type for the given entity.
  virtual EntityType GetEntityType() const { return kEntityTypeOperator; }

  // The combiner associated with the operator if any.
  string combiner;

  SERIALIZE_VIRTUAL(DEFAULT_CUSTOM / eid*1 / combiner*2);
};

}  // namespace op
}  // namespace entity

#endif  // _UTIL_ENTITY_OPERATORS_ENTITY_OPERATORS_H_
