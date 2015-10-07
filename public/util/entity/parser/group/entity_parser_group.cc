// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// This algo maps each entity type to a specific parser.

#include "util/entity/parser/group/entity_parser_group.h"

#include <memory>

#include "base/logging.h"

namespace entity {
namespace parser {

bool EntityParserGroup::Initialize() {
  LOG(INFO) << "Initializing EntityParserGroup with params: " << params().ToJSON();

  for (const EntityParserGroupParams::ParserParams& param : params().parser_params) {
    EntityParser::shared_proxy proxy = EntityParser::make_shared(param.parser_id,
                                                                 param.parser_params);
    if (proxy == nullptr) {
      LOG(ERROR) << "Could not init parser: " << param.ToJSON();
      return false;
    }
    entity_parser_map_[param.entity_type] = proxy;
  }

  if (entity_parser_map_.empty()) {
    LOG(ERROR) << "No parser found. Invalid Params: " << params().ToJSON();
    return false;
  }

  LOG(INFO) << "Initialized EntityParserGroup with " << entity_parser_map_.size() << " parsers.";
  return true;
}

const tEntity* EntityParserGroup::Parse(const string& eid) const {
  EntityType type = GetEntityTypeFromEntityId(eid);
  if (type == kEntityTypeInvalid) return nullptr;

  const tEntity* entity = nullptr;
  const auto iter = entity_parser_map_.find(type);
  if (iter != entity_parser_map_.end()) entity = iter->second->Parse(eid);

  return entity;
}

}  // namespace parser
}  // namespace entity
