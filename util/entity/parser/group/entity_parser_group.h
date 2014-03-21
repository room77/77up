// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// This algo allows a group of parsers to be registerd for different entity types.

#ifndef _UTIL_ENTITY_PARSER_GROUP_ENTITY_PARSER_GROUP_H_
#define _UTIL_ENTITY_PARSER_GROUP_ENTITY_PARSER_GROUP_H_

#include "util/entity/parser/entity_parser.h"

#include <unordered_map>

#include "base/defs.h"
#include "util/entity/entity_id.h"
#include "util/serial/serializer.h"
#include "util/serial/types/arbit_blob.h"

namespace entity {
namespace parser {

// Class that allows a group of parsers to be registerd for different entity types.
class EntityParserGroup : public EntityParser {
 public:
  virtual ~EntityParserGroup() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) {
    return params_.FromJSON(opts);
  }

  // Initialize the class.
  virtual bool Initialize();

  // Returns the entity for the corresponding entity id.
  // If the entity cannot be found for the entity id, nullptr is returned.
  virtual const tEntity* Parse(const string& eid) const;

 protected:
  // The parameters struct to configure the algo.
  struct EntityParserGroupParams {
    struct ParserParams {
      // The entity type associated with the parser.
      EntityType entity_type;

      // The id of the parser to use.
      string parser_id;

      // The params to be passed to the parser while generating the parser.
      // This can be any arbit blob.
      serial::types::ArbitBlob parser_params;

      SERIALIZE(DEFAULT_CUSTOM / entity_type*1 / parser_id*2 / parser_params*3);
    };

    // List of parsers.
    vector<ParserParams> parser_params;

    SERIALIZE(DEFAULT_CUSTOM / parser_params*2);
  };

  const EntityParserGroupParams& params() const { return params_; }

  EntityParserGroupParams params_;

  // Map from entity prefix -> entity manager proxy.
  unordered_map<EntityType, EntityParser::shared_proxy> entity_parser_map_;
};

}  // namespace parser
}  // namespace entity


#endif  // _UTIL_ENTITY_PARSER_GROUP_ENTITY_PARSER_GROUP_H_
