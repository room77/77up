// Copyright 2012 Room77, Inc.
// Author: pramodg@room77.com (Pramod Gupta)

// This file is for setting up a test server for region data.

#include "util/init/main.h"
#include "util/network/rpcserver.h"
#include "util/network/method/server_method.h"
#include "util/geo/latlong.h"
#include "util/region_data/attractions.h"
#include "util/region_data/cities.h"
#include "util/region_data/countries.h"
#include "util/region_data/states.h"

FLAG_int(port, 10001, "server port number");

// RPC function definitions.
const char kRegionDataTestServer[] = "region_data_test_server";

namespace region_data {

template <typename Data>
struct tTestReply {
  vector<Data> list;
  SERIALIZE(list*1);
};

struct tTestRadiusSearchRequest {
  double lat;
  double lon;
  float max_radius;       // In kilometers.
  int max_results;
  SERIALIZE(lat*1 / lon*2 / max_radius*3 / max_results*4);
};

template<typename Data>
struct tTestRadiusSearchReply {
  vector<pair<Data, double> > list;
  SERIALIZE(list*1);
};

// Country API.
typedef tTestReply<tCountry> tCountryTestReply;
typedef tTestRadiusSearchReply<tCountry> tCountrytTestRadiusSearchReply;

class ParseCountry : public network::ServerMethod {
 public:
  string operator()(const string& input, tCountryTestReply* result) const {
    const static int kMaxRes = 1000;
    region_data::Countries::Instance().ParseAndConvert(input, &(result->list),
        kMaxRes, ptr_to_data<region_data::tCountry>());
    return "";
  }
};
network::ServerMethodRegister<string, tCountryTestReply, ParseCountry>
    reg_ParseCountry(kRegionDataTestServer, "ParseCountry", "US");

class CountryName : public network::ServerMethod {
 public:
  string operator()(const string& input, tCountryTestReply* result) const {
    const static int kMaxCountries = 1000;
    vector<const region_data::tCountry*> res;
    region_data::Countries::Instance().LookupByName(input, &res, kMaxCountries);
    result->list.reserve(res.size());
    for (const region_data::tCountry* r : res) {
      result->list.push_back(*r);
    }
    return "";
  }
};
network::ServerMethodRegister<string, tCountryTestReply, CountryName>
    reg_CountryName(kRegionDataTestServer, "CountryName", "United States");

class RadiusSearchCountry : public network::ServerMethod {
  public:
    string operator()(const tTestRadiusSearchRequest& req,
                      tCountrytTestRadiusSearchReply* reply) const {
      vector<pair<const region_data::tCountry*, double> > res;
      LatLong ll = LatLong::Create(req.lat, req.lon);
      region_data::Countries::Instance().LookupByLatLong(ll, &res,
                                                          req.max_results,
                                                          req.max_radius);

      reply->list.reserve(res.size());
      for (const auto& pair : res) {
        reply->list.push_back(make_pair(*(pair.first), pair.second));
      }
      return "";
  }
};
network::ServerMethodRegister<tTestRadiusSearchRequest, tCountrytTestRadiusSearchReply, RadiusSearchCountry>
    reg_RadiusSearchCountry(kRegionDataTestServer, "RadiusSearchCountry",
                            { 48.8579, 2.2949, 5000, 10 });

// City API.
typedef tTestReply<tCity> tCityTestReply;
typedef tTestRadiusSearchReply<tCity> tCitytTestRadiusSearchReply;

class ParseCity : public network::ServerMethod {
 public:
  string operator()(const string& input, tCityTestReply* result) const {
    const static int kMaxAttractions = 1000;
    region_data::Cities::Instance().ParseAndConvert(input, &(result->list),
        kMaxAttractions, ptr_to_data<region_data::tCity>());
    return "";
  }
};
network::ServerMethodRegister<string, tCityTestReply, ParseCity>
    reg_ParseCity(kRegionDataTestServer, "ParseCity", "san fr");

class CityName : public network::ServerMethod {
 public:
  string operator()(const string& input, tCityTestReply* result) const {
    const static int kMaxCities = 1000;
    vector<const region_data::tCity*> res;
    region_data::Cities::Instance().LookupByName(input, &res, kMaxCities);
    result->list.reserve(res.size());
    for (const region_data::tCity* r : res) {
      result->list.push_back(*r);
    }
    return "";
  }
};
network::ServerMethodRegister<string, tCityTestReply, CityName>
    reg_CityName(kRegionDataTestServer, "CityName", "san francisco");

class RadiusSearchCity : public network::ServerMethod {
  public:
    string operator()(const tTestRadiusSearchRequest& req,
                      tCitytTestRadiusSearchReply* reply) const {
      vector<pair<const region_data::tCity*, double> > res;
      LatLong ll = LatLong::Create(req.lat, req.lon);
      region_data::Cities::Instance().LookupByLatLong(ll, &res,
                                                          req.max_results,
                                                          req.max_radius);

      reply->list.reserve(res.size());
      for (const auto& pair : res) {
        reply->list.push_back(make_pair(*(pair.first), pair.second));
      }
      return "";
  }
};
network::ServerMethodRegister<tTestRadiusSearchRequest, tCitytTestRadiusSearchReply, RadiusSearchCity>
    reg_RadiusSearchCity(kRegionDataTestServer, "RadiusSearchCity",
                            { 48.8579, 2.2949, 5000, 10 });

// Attractions API.
typedef tTestReply<tAttraction> tAttractionTestReply;
typedef tTestRadiusSearchReply<tAttraction> tAttractionTestRadiusSearchReply;

class ParseAttraction : public network::ServerMethod {
 public:
  string operator()(const string& input, tAttractionTestReply* result) const {
    const static int kMaxRes = 50;
    region_data::Attractions::Instance().ParseAndConvert(input, &(result->list),
        kMaxRes, ptr_to_data<region_data::tAttraction>());
    return "";
  }
};
network::ServerMethodRegister<string, tAttractionTestReply, ParseAttraction>
    reg_ParseAttraction(kRegionDataTestServer, "ParseAttraction", "Eiffel");

class AttractionName : public network::ServerMethod {
 public:
  string operator()(const string& input, tAttractionTestReply* result) const {
    const static int kMaxRes = 50;
    vector<const region_data::tAttraction*> res;
    region_data::Attractions::Instance().LookupByName(input, &res,
                                                       kMaxRes);
    result->list.reserve(res.size());
    for (const region_data::tAttraction* r : res) {
      result->list.push_back(*r);
    }
    return "";
  }
};
network::ServerMethodRegister<string, tAttractionTestReply, AttractionName>
    reg_AttractionName(kRegionDataTestServer, "AttractionName", "Eiffel Tower");

struct tRetrieveAttractionRequest {
  string name, city, state_code, country_code;
  SERIALIZE(name*1 / city*2 / state_code*3 / country_code*4);
};

class RetrieveAttraction : public network::ServerMethod {
  public:
    string operator()(const tRetrieveAttractionRequest& req,
                      tAttractionTestReply* reply) const {
      const static int kMaxRes = 10;
      vector<const region_data::tAttraction*> res;
      region_data::Attractions::Instance().LookupByNameEx(req.name, req.city,
          req.state_code, req.country_code, &res, kMaxRes);

      reply->list.reserve(res.size());
      for (const region_data::tAttraction* r : res) {
        reply->list.push_back(*r);
      }
      return "";
  }
};
network::ServerMethodRegister<tRetrieveAttractionRequest, tAttractionTestReply, RetrieveAttraction>
    reg_RetrieveAttraction(kRegionDataTestServer, "RetrieveAttraction",
                           { "Eiffel Tower", "Paris", "", "FR" });

class RadiusSearchAttraction : public network::ServerMethod {
  public:
    string operator()(const tTestRadiusSearchRequest& req,
                      tAttractionTestRadiusSearchReply* reply) const {
      vector<pair<const region_data::tAttraction*, double> > res;
      LatLong ll = LatLong::Create(req.lat, req.lon);
      region_data::Attractions::Instance().LookupByLatLong(ll, &res,
                                                            req.max_results,
                                                            req.max_radius);

      reply->list.reserve(res.size());
      for (const auto& pair : res) {
        reply->list.push_back(make_pair(*(pair.first), pair.second));
      }
      return "";
  }
};
network::ServerMethodRegister<tTestRadiusSearchRequest, tAttractionTestRadiusSearchReply, RadiusSearchAttraction>
    reg_RadiusSearchAttraction(kRegionDataTestServer, "RadiusSearchAttraction",
                               { 48.8579, 2.2949, 10000, 10 });

}  // namespace region_data

int init_main() {
  using namespace region_data;

  ASSERT(gFlag_port > 0) << "port number must be positive!";

  // region_data::Attractions::Instance();
  region_data::Cities::Instance();

  network::RPCServer server(kRegionDataTestServer);

  server.set_portnum(gFlag_port);
  server.StartServer();

  return 0;
}
