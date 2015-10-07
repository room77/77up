#include "util/entity/parser/group/entity_parser_groups_util.h"

extern string gFlag_entity_parser_group_locations;

namespace entity {
namespace parser {
namespace util {

const region_data::tRegion* GetRegionByEid(const string& eid) {
  using ::entity::parser::EntityParser;
  static EntityParser::shared_proxy location_parser =
      EntityParser::make_shared(gFlag_entity_parser_group_locations);
  ASSERT_NOTNULL(location_parser);
  return dynamic_cast<const region_data::tRegion *>(location_parser->Parse(eid));
}

}  // namespace util
}  // namespace parser
}  // namespace entity
