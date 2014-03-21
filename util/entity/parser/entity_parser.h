// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Basic interface for entity parsers.

#ifndef _UTIL_ENTITY_PARSER_ENTITY_PARSER_H_
#define _UTIL_ENTITY_PARSER_ENTITY_PARSER_H_

#include "base/defs.h"
#include "util/entity/entity.h"
#include "util/factory/factory.h"

namespace entity {
namespace parser {

// The base class for all entity parsers.
class EntityParser : public Factory<EntityParser, string, string> {
 public:
  virtual ~EntityParser() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() { return true; }

  // Returns the entity for the corresponding entity id.
  // If the entity cannot be found for the entity id, nullptr is returned.
  virtual const tEntity* Parse(const string& eid) const = 0;
};

}  // namespace parser
}  // namespace entity


#endif  // _UTIL_ENTITY_PARSER_ENTITY_PARSER_H_
