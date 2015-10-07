// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/entity/parser/entity_parser.h"

#include <memory>

#include "util/entity/entity_manager.h"
#include "util/factory/factory_extra.h"
#include "util/serial/serializer.h"
#include "util/serial/types/arbit_blob.h"

namespace entity {
namespace parser {

// Class that searches an entity manager to parse a given entity.
class EntityParserEM : public EntityParser {
 public:
  virtual ~EntityParserEM() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return params_.FromJSON(opts); }

  // Initialize the class.
  virtual bool Initialize();

  // Returns the entity for the corresponding entity id.
  // If the entity cannot be found for the entity id, nullptr is returned.
  virtual const tEntity* Parse(const string& eid) const;

 protected:
  struct EntityParserEMParams {
    // The id for the entity manager.
    string em_id;

    // The params for the entity manager.
    serial::types::ArbitBlob em_params;

    SERIALIZE(DEFAULT_CUSTOM / em_id*1 / em_params*2);
  };

  const EntityParserEMParams& params() const { return params_; }

  EntityParserEMParams params_;

  EntityManager::shared_proxy em_proxy_;
};

bool EntityParserEM::Initialize() {
  LOG(INFO) << "Initializing EntityParserEM with params: " << params().ToJSON();

  if (params().em_id.empty()) {
    LOG(ERROR) << "Must specify em_id: " << params().ToJSON();
    return false;
  }

  em_proxy_ = params().em_params.empty() ? EntityManager::make_shared(params().em_id) :
      EntityManager::make_shared(params().em_id, params().em_params);

  if (em_proxy_ == nullptr) {
    LOG(ERROR) << "Could not init parser: " << params().ToJSON();
    return false;
  }

  LOG(INFO) << "Initialized EntityEMParser with " << params().ToJSON();
  return true;
}

const tEntity* EntityParserEM::Parse(const string& eid) const {
  return eid.empty() ? nullptr : em_proxy_->LookupUniqueByEntityId(eid);
}

// Register entity_em_parser.
// Notes:
// 1. The default parameters are invalid. We expect the client to specify then em_id during
//    make_shared to get the right entity manager.
// 2. The client is responsible for linking in the apporopriate dependencies.
auto reg_entity_parser_em = EntityParser::bind("entity_parser_em", "",
    InitializeConfigureConstructor<EntityParserEM, string>());

}  // namespace parser
}  // namespace entity
