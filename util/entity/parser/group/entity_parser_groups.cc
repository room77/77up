// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


// Register a few default groups.

#include "util/entity/parser/group/entity_parser_group.h"

#include <memory>

#include "base/args/args.h"
#include "util/factory/factory_extra.h"

FLAG_string(entity_parser_group_all, "entity_parser_group_all",
            "factory key for entity parser group for all entities");

FLAG_string(entity_parser_group_locations, "entity_parser_group_locations",
            "factory key for entity parser group for location entities");

FLAG_string(entity_parser_group_all_params,
            "{"
              "\"parser_params\": "
              "["
                "{"
                  "\"entity_type\": " + to_string(entity::kEntityTypeCity) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_cities\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeState) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_states\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeCountry) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_countries\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeHotel) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_hotelmanager\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeAttraction) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_attractions\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeAirport) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_airports\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeNeighborhood) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_neighborhoods\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeAmenity) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_attribute_amenity\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeFilter) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_attribute_filter\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeRanker) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_attribute_sort\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeOperator) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_operators\","
                  "},"
                "}"
              "]"
            "}",
            "Parameters for the group registering all parsers.");

FLAG_string(entity_parser_group_locations_params,
            "{"
              "\"parser_params\": ["
                "{"
                  "\"entity_type\": " + to_string(entity::kEntityTypeCity) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_cities\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeState) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_states\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeCountry) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_countries\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeHotel) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_hotelmanager\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeAttraction) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_attractions\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeAirport) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_airports\","
                  "},"
                "}, {"
                  "\"entity_type\": " + to_string(entity::kEntityTypeNeighborhood) + ","
                  "\"parser_id\": \"entity_parser_em\","
                  "\"parser_params\": {"
                    "\"em_id\": \"em_neighborhoods\","
                  "},"
                "}"
              "]"
            "}",
            "Parameters for the group registering parsers that identify locations.");

namespace entity {
namespace parser {

// Register entity_em_parser.
auto reg_entity_parser_group_all = EntityParser::bind(
    gFlag_entity_parser_group_all, gFlag_entity_parser_group_all_params,
    InitializeConfigureConstructor<EntityParserGroup, string>());

// Register entity_em_parser.
auto reg_entity_parser_group_locations = EntityParser::bind(
    gFlag_entity_parser_group_locations, gFlag_entity_parser_group_locations_params,
    InitializeConfigureConstructor<EntityParserGroup, string>());

}  // namespace parser
}  // namespace entity
