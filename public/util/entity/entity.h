// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_ENTITY_ENTITY_H_
#define _UTIL_ENTITY_ENTITY_H_

#include "base/defs.h"
#include "util/entity/entity_type.h"
#include "util/serial/serializer.h"

namespace entity {

// A common base class that all entities should derive from.
struct tEntity {
  virtual ~tEntity() {}

  // Returns the entity type for the given entity. All subclasses must correctly set this value.
  virtual EntityType GetEntityType() const { return kEntityTypeInvalid; }

  virtual const string& GetEid() const { return eid; }
  virtual void SetEid(const string& id) { eid = id; }

  // The entity id for the entity. This should be enough to uniquely identify the entity *globally*.
  string eid;

  SERIALIZE_VIRTUAL(DEFAULT_CUSTOM / eid*1);
};

}  // namespace entity


#endif  // _UTIL_ENTITY_ENTITY_H_
