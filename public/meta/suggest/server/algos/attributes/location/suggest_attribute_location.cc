// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/algos/attributes/location/suggest_attribute_location.h"

#include "base/defs.h"
#include "util/string/strutil.h"
#include "util/entity/entity_id.h"

FLAG_string(distance_eid, "r/Distance", "The distance ranker id.");
FLAG_string(neighborhood_eid, "f/Neighborhood", "The neighborhood ranker id.");

namespace suggest {
namespace algo {

void SuggestLocationAttribute::PrepareChildCompletionFromParent(const Completion& parent,
                                                                Completion* child) const {
  static const string kOpParamBegin =
      ::entity::GetEntityIdFromBaseId(entity::kEntityTypeOperator, "pb");
  static const string kOpParamEnd =
      ::entity::GetEntityIdFromBaseId(entity::kEntityTypeOperator, "pe");

  // TODO(pramodg, sungcho): Use the right denom once the put the absolute score for the child
  // instead of the ratio that we have currently.
  // double denom = parent.suggestion->base_score == 0 ? 1.0 : parent.suggestion->base_score;
  double denom = 1;
  double multiplicative_factor = parent.score / denom;

  child->score *= multiplicative_factor;

  // We need to ensure that the suggestion id is always unique.
  // Refer T2385 for the format.
#if 0
  child->suggestion_id = strutil::JoinString({child->suggestion_id, kOpParamBegin,
      '"' + child->suggestion_id + '"', kOpParamEnd, gFlag_distance_eid},
      ::entity::GetMultipleEntitiesSeparator());
#endif

#if 1
  const auto ranker_filter_eid =
      (child->suggestion->src_type == ::entity::kEntityTypeNeighborhood)
      ? gFlag_neighborhood_eid : gFlag_distance_eid;
  child->suggestion_id = strutil::JoinString({
        parent.suggestion_id,
        kOpParamBegin,
        '"' + child->suggestion_id + '"',
        kOpParamEnd,
        ranker_filter_eid},
      ::entity::GetMultipleEntitiesSeparator());
#endif
}

}  // namespace algo
}  // namespace suggest
