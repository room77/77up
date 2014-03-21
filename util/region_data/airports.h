/*
  Copyright 2013, Room 77, Inc.
  @author Kyle Konrad <kyle@room77.com>

  A Region interface for airports
*/

#ifndef _UTIL_REGION_DATA_AIRPORTS_H_
#define _UTIL_REGION_DATA_AIRPORTS_H_

#include "base/defs.h"
#include "util/region_data/region.h"

extern string gFlag_travel_ports_file;
extern string gFlag_airport_names_file_dev_only;
extern string gFlag_airport_city_codes_file_dev_only;

namespace region_data {

struct tAirport : public region_data::tRegion {
  string code;
  string countrycode;
  string statecode;
  string metrocode;
  string citycode;
  string phoneprefix;
  int type = -1;

  // Galileo database says it's a major airport (but it may not be)
  inline bool IsMajorAirport() const {
    return (type == 1 || type == 2);
  }

  virtual entity::EntityType GetEntityType() const {
    return entity::kEntityTypeAirport;
  }

  CSV(eid | code | name | countrycode | statecode | metrocode | citycode |
      phoneprefix | type | lat | lon);

  SERIALIZE_VIRTUAL(eid*1 / code*2 / name*3 / countrycode*4 / statecode*5 /
                    metrocode*6 / citycode*7 / phoneprefix*8 / type*9 /
                    lat*10 / lon*11);
};

struct order_airports_by_code {
  bool operator()(const tAirport& a, const tAirport& b) {
    return a.code < b.code;
  }
  bool operator()(const tAirport *a, const tAirport *b) {
    return operator()(*a, *b);
  }
};

class Airports : public Region<tAirport, order_airports_by_code> {
  typedef Region<tAirport, order_airports_by_code> super;

 public:
  virtual ~Airports() {}

  virtual bool Initialize();

  static Airports& Instance();

  virtual const tAirport *LookupAirportByCode(const string& code) const;

 protected:
  // Declare this class as friend so that it call its constructor.
  friend class InitializeConfigureConstructor<Airports, string>;
  Airports() = default;
};
} // namespace region_data

#endif // _UTIL_REGION_DATA_AIRPORTS_H_
