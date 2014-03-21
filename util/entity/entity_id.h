// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Defines a globally unique entity id for each entity known to us.
// For example:
//   A city with id [xyz] will have entity id [c/xyz].
//   A hotel with id [1234] will have entity id [h/1234].


#ifndef _UTIL_ENTITY_ENTITY_ID_H_
#define _UTIL_ENTITY_ENTITY_ID_H_

#include <sstream>

#include "base/defs.h"
#include "util/string/strutil.h"
#include "util/entity/entity_type.h"
#include "util/templates/container_util.h"

namespace entity {

// Returns the separator that separates the entity prefix from the entity base
// id.
inline const string& GetEntityIdPrefixSeparator() {
  static const string kEntityIdPrefixSeparator = "/";
  return kEntityIdPrefixSeparator;
}

// Returns the separator that separates multiple contiguous entities.
inline const string& GetMultipleEntitiesSeparator() {
  static const string kMultipleEntitiesSeparator = "|";
  return kMultipleEntitiesSeparator;
}

// Generates the entity id from the base id and the entity type.
template <typename T>
inline string GetEntityIdFromBaseId(const EntityType& type, const T& base_id) {
  ostringstream ss;
  ss << GetEntityPrefixForType(type) << GetEntityIdPrefixSeparator() << base_id;
  return ss.str();
}

// Parses the base id and the entity type from the entity id.
template <typename T>
inline EntityType GetBaseIdAndTypeFromEntityId(const string& entity_id,
                                               T* base_id) {
  vector<string> parts;
  if (strutil::SplitString(entity_id, GetEntityIdPrefixSeparator(),
                              &parts) != 2) return kEntityTypeInvalid;

  *base_id = strutil::FromString<T>(parts[1]);
  return GetEntityTypeFromPrefix(parts[0]);
}

// Parses the base id from the entity id. We assume that the default value of type T is an
// invalid value and can be used to check if the base id is valid or not.
template <typename T>
inline T GetBaseIdFromEntityId(const string& entity_id) {
  vector<string> parts;
  if (strutil::SplitString(entity_id, GetEntityIdPrefixSeparator(),
                              &parts) != 2) return T();
  return strutil::FromString<T>(parts[1]);
}

// Parses the entity type from the entity id.
inline EntityType GetEntityTypeFromEntityId(const string& entity_id) {
  vector<string> parts;
  if (strutil::SplitString(entity_id, GetEntityIdPrefixSeparator(),
                              &parts) != 2) return kEntityTypeInvalid;

  return GetEntityTypeFromPrefix(parts[0]);
}

// Appends the from entity to another.
inline string& AppendEntityId(const string& to_add, string* id) {
  id->append(GetMultipleEntitiesSeparator());
  id->append(to_add);
  return *id;
}

inline string JoinEntityIds(const string& left, string right) {
  return strutil::JoinTwoStrings(left, right, GetMultipleEntitiesSeparator());
}

template<typename C = vector<string>>
inline C SplitEntityIds(const string& eid) {
  C res;
  strutil::SplitString(eid, GetMultipleEntitiesSeparator(), &res);
  return res;
}

}  // namespace entity


#endif  // _UTIL_ENTITY_ENTITY_ID_H_
