/*
  Copyright 2013, Room 77, Inc.
  @author Kyle Konrad <kyle@room77.com>

  A Region interface for airports
*/

#include "util/init/init.h"
#include "util/region_data/airports.h"
#include "util/entity/entity_id.h"

FLAG_string(em_airports_id, "em_airports",
            "The default entity manager id for airports.");

FLAG_string(region_airports_file,
            "static_data/push/auto/region_data/airports/airports.csv",
            "List of airports");

FLAG_string(travel_ports_file,
            "static_data/dev_only/region_data/airports/travel_ports.txt",
            "world airport database");
FLAG_string(airport_names_file_dev_only,
            "static_data/dev_only/region_data/airports/airport_names.txt",
            "dev_only airport names");
FLAG_string(airport_city_codes_file_dev_only,
            "static_data/dev_only/region_data/airports/airport_city_codes.txt",
            "dev_only airport city codes");

namespace region_data {

bool Airports::Initialize() {
  // Index by name, prefix and latlong.
  super::ConfigParams params;
  params.file = gFlag_region_airports_file;
  params.build_name_index = true;
  params.name_index_is_unique = false;
  params.build_latlong_index = true;
  params.build_entity_id_index = true;

  ASSERT(super::Initialize(params));
  return true;
}

Airports& Airports::Instance() {  // singleton instance
  struct Creator {
    Airports* CreateAirports() {
      LOG(INFO) << "Creating Airports using Id: " << gFlag_em_airports_id;
      mutable_shared_proxy proxy = make_shared(gFlag_em_airports_id);
      ASSERT_NOTNULL(proxy);
      pin(proxy);
      return dynamic_cast<Airports*>(proxy.get());
    }
  };
  static Airports* the_one = Creator().CreateAirports();
  return *the_one;
}

const tAirport *Airports::LookupAirportByCode(const string& code) const {
  return LookupUniqueByEntityId(
      entity::GetEntityIdFromBaseId(entity::kEntityTypeAirport, code));
}

// Register the airports with the enity manager.
auto reg_em_airports = ::entity::EntityManager::bind(gFlag_em_airports_id, "",
    InitializeConfigureConstructor<Airports, string>());

}  // namespace region_data

// Init before code translator.
INIT_ADD("airports", 0, []{ region_data::Airports::Instance(); });
