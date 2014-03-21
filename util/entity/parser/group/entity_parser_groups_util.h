#ifndef _UTIL_ENTITY_PARSER_GROUP_ENTITY_PARSER_GROUPS_UTIL_H_
#define _UTIL_ENTITY_PARSER_GROUP_ENTITY_PARSER_GROUPS_UTIL_H_

#include "util/entity/parser/entity_parser.h"
#include "util/region_data/region.h"

namespace entity {
namespace parser {
namespace util {

// Get a region using the default entity parser group for locations.
const region_data::tRegion* GetRegionByEid(const string& eid);

}  // namespace util
}  // namespace parser
}  // namespace entity

#endif  // _UTIL_ENTITY_PARSER_GROUP_ENTITY_PARSER_GROUPS_UTIL_H_
