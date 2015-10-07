// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _UTIL_ENTITY_ENTITY_MANAGER_H_
#define _UTIL_ENTITY_ENTITY_MANAGER_H_

#include "base/defs.h"
#include "util/entity/entity.h"
#include "util/factory/factory.h"
#include "util/factory/factory_extra.h"

namespace entity {

// A common base class that all things that manage any entities.
class EntityManager : public Factory<EntityManager, string, string>  {
 public:
  virtual ~EntityManager() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  // Returns the entity for the given entity id.
  // All subclasses should implement this function to return the right entity.
  // If the entity cannot be found for the id, nullptr should be returned.
  // The default implementation return nullptr.
  virtual const tEntity* LookupUniqueByEntityId(const string& entity_id) const {
    return nullptr;
  }

  // Returns a mutable entity for the given entity id.
  // This should be used very very carefully.
  virtual tEntity* LookupMutableByEntityId(const string& entity_id) const {
    return const_cast<tEntity*>(LookupUniqueByEntityId(entity_id));
  }
};

}  // namespace entity


#endif  // _UTIL_ENTITY_ENTITY_MANAGER_H_
