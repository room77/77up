// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/log/offline/utils/location_utils.h"

#include <algorithm>

#include "util/entity/entity_id.h"
#include "util/entity/entity_type.h"
#include "util/region_data/region_utils.h"
#include "util/templates/container_util.h"

namespace logging {
namespace offline {
namespace utils {

vector<string> GetLocationIdsFromSgstId(const string& suggest_id) {
  vector<string> res;
  strutil::SplitString(suggest_id, entity::GetMultipleEntitiesSeparator(), &res);

  auto iter = remove_if(res.begin(), res.end(), [](const string& id) -> bool {
    entity::EntityType type = entity::GetEntityTypeFromEntityId(id);
    // Only keep GEO or POI type. Remove all others.
    if (entity::IsTypeGeo(type) || entity::IsTypePOI(type)) return false;

    return true;
  });

  // Resize if necessary
  if (iter != res.end()) res.resize(iter - res.begin());

  return res;
}

vector<string> GetLocationIds(const tLogElement& element, bool fill_key_if_none_found) {
  static const string kEmptyString;
  const string sgst_id = ::util::tl::FindWithDefault(element.cgi_params, "sgst_id", kEmptyString);

  if (!sgst_id.empty()) {
    entity::EntityType type = entity::GetEntityTypeFromEntityId(sgst_id);
    // If the entity type is city, we want to ignore deprecated id which were like c/US:14331423.
    if (type != entity::kEntityTypeCity || sgst_id.find(':') == string::npos)
      return GetLocationIdsFromSgstId(sgst_id);
  }

  vector<string> res;
  string key = ::util::tl::FindWithDefault(element.cgi_params, "key", kEmptyString);
  if (key.empty()) return res;

  string lat = ::util::tl::FindWithDefault(element.cgi_params, "lat", "0");
  string lon = ::util::tl::FindWithDefault(element.cgi_params, "lon", "0");

  string id = ::region_data::utils::GetLocationIdFromName(key, strutil::ParseDouble(lat),
                                                          strutil::ParseDouble(lon));
  if (id.size()) res.push_back(id);
  else if (fill_key_if_none_found) res.push_back(key);

  return res;
}

}  // namespace utils
}  // namespace offline
}  // namespace logging

