// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/entity/operators/entity_operators.h"

#include "util/entity/entity_id.h"
#include "util/entity/entity_manager.h"
#include "util/factory/factory_extra.h"

namespace entity {
namespace op {

// The manager for different operators that can be present as part of the entity id.
class OperatorsManager : public EntityManager  {
 public:
  virtual ~OperatorsManager() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }

  // Initialize the class.
  virtual bool Initialize() {
    AddBasicOperator("+");  // [SF or Berkeley], [SF and Berkeley].
    AddBasicOperator("&");  // [SF cheap and 3 star]
    AddBasicOperator("~");  // [SF cheap not 1 star], [SF far from Alcatraz]

    // Everything between pb and pe is treated as params to the following entity (usually a ranker).
    AddBasicOperator("pb");
    AddBasicOperator("pe");
    return true;
  }

  // Returns the entity for the given entity id.
  // All subclasses should implement this function to return the right entity.
  // If the entity cannot be found for the id, nullptr should be returned.
  // The default implementation return nullptr.
  virtual const tOperator* LookupUniqueByEntityId(const string& entity_id) const {
    auto const iter = operators_.find(entity_id);
    if (iter != operators_.end()) return &(iter->second);
    return nullptr;
  }

 private:
  void AddBasicOperator(const string& op_id) {
    tOperator op;
    op.combiner = op_id;
    op.eid = GetEntityPrefixForType(kEntityTypeOperator) + GetEntityIdPrefixSeparator() + op_id;
    operators_[op.eid] = op;
  }

  // Map from the eid -> to the Operator data.
  unordered_map<string, tOperator> operators_;
};

// Register the operators with the enity manager.
auto reg_em_operators = ::entity::EntityManager::bind("em_operators", "",
    InitializeConfigureConstructor<OperatorsManager, string>());

}  // namespace op
}  // namespace entity
