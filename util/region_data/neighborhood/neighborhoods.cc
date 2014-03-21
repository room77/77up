// Copyright 2013 Room77, Inc.
// Author: akearney@room77.com (Andy Kearney), pramodg@room77.com (Pramod Gupta)

#include "util/region_data/neighborhood/neighborhoods.h"

#include <algorithm>

#include "util/init/init.h"
#include "util/string/strutil.h"
#include "util/file/filereader.h"
#include "util/geo/latlong.h"
#include "util/region_data/utils.h"
#include "util/templates/comparator.h"
#include "util/time/simple_timer.h"

FLAG_string(em_neighborhoods_id, "em_neighborhoods",
            "The default entity manager id for neighborhoods.");

// Maponics info
FLAG_string(hotel_to_location_file,
            "/home/share/data/search/maponics/current/hid_to_pid.txt",
            "File containing mapping from hotel id to maponics location id");

FLAG_string(maponics_index_file,
            "/home/share/data/maponics_static/current/index_info.dat",
            "File containing the sizing for indices for maponics");

FLAG_string(maponics_location_file,
            "/home/share/data/search/maponics/current/pid_to_tLoc.txt",
            "File containing mapping from maponics location id to location info");

struct IndexHelp;

namespace region_data {

tNeighborhoodInfoReply neighborhood_storage_to_reply::operator() (
    const tNeighborhoodInfoWithBounds& nbr) const {
  tNeighborhoodInfoReply reply = super::operator()(nbr);
  // City Info.
  reply.mcd = nbr.loc.mcd;
  reply.name = nbr.loc.name;
  reply.id = nbr.loc.id;
  reply.latitude = nbr.get_latitude();
  reply.longitude = nbr.get_longitude();
  reply.zoom_level = nbr.loc.zoom_level;

  // State Info.
  // TODO(akearney): country codes
  reply.state = nbr.loc.state;
  reply.hotel_count = nbr.loc.hotel_count;

  return reply;
}

bool order_neighborhoods_by_name::operator() (const tNeighborhoodInfoWithBounds& n1,
                                              const tNeighborhoodInfoWithBounds& n2) const {
  if (n1.loc.name != n2.loc.name) return n1.loc.name < n2.loc.name;

  if (n1.loc.mcd != n2.loc.mcd) return n1.loc.mcd < n2.loc.mcd;
  // Order neighborhoods with same name by state.
  if (n1.loc.state != n2.loc.state) return n1.loc.state < n2.loc.state;

  // Break ties arbitrarily.
  return n1.loc.id < n2.loc.id;
}

bool Neighborhoods::Initialize() {
  // Index by name, prefix and latlong.
  // latlong is maintained internally.
  LOG(INFO) << "Initializing Neighborhoods";
  super::ConfigParams params;
  params.file = gFlag_maponics_location_file;
  params.create_entity_ids = true;
  params.build_entity_id_index = true;
  params.build_name_index = false;
  params.name_index_is_unique = false;

  // Note: The Neighborhoods maintain their own latlong index because it
  // behaves slightly differently than the Attractions for instance. There
  // are indices for each zoom level as opposed to just 1 level since the
  // grid size is VERY different between the largest and the smallest levels.
  params.build_latlong_index = false;

  LOG(INFO) << "Reading neighborhoods.";
  ::util::time::SimpleTimer timer;
  timer.Start();

  ASSERT(super::Initialize(params));
  BuildIndices();

  timer.Stop();
  LOG(INFO) << "Finished reading " << storage_.size() << " neighborhoods. Took "
         << timer.GetDurationSec() << " seconds.";
  return true;
}

void Neighborhoods::BuildIndices() {
  BuildNameIndex();
  BuildLatLongIndex();
  BuildIdToLocationMap();
  BuildHidToLocationMap();
}

// Initializes the storage.
int Neighborhoods::InitStorage(const string& file) {
  ASSERT(!file.empty());
  FileReader::ProcessAll<tNeighborhoodInfoWithBounds>(file,
      [this](tNeighborhoodInfoWithBounds&& nbr) {
        storage_.emplace_back(std::make_shared<tNeighborhoodInfoWithBounds>(nbr));
      }, true, FileReader::BinaryParser<tNeighborhoodInfoWithBounds>());

  LOG(INFO) << "Read " << storage_.size() << " neighborhoods.";
  return storage_.size();
}

void Neighborhoods::BuildNameIndex() {
  for (const shared_ptr<tNeighborhoodInfoWithBounds>& nbr : storage()) {
    // Index by name.
    name_index_.AddToIndex(utils::NormalizeString(nbr->loc.name), nbr.get(),
                           params_.name_index_is_unique);

    // Index by name and city.
    if (!nbr->loc.mcd.empty())
      name_index_.AddToIndex(utils::NormalizeString(nbr->loc.name + " " + nbr->loc.mcd), nbr.get(),
                             params_.name_index_is_unique);

    // Index by name, city and state.
    if (!nbr->loc.state.empty())
      name_index_.AddToIndex(utils::NormalizeString(nbr->loc.name + " " + nbr->loc.mcd + " " +
          nbr->loc.state), nbr.get(), params_.name_index_is_unique);
  }
}

void Neighborhoods::BuildLatLongIndex() {
  // Note: The Neighborhoods maintain their own latlong index because it
  // behaves slightly differently than the Attractions for instance. There
  // are indices for each zoom level as opposed to just 1 level since the
  // grid size is VERY different between the largest and the smallest levels.
  BuildZoomToIndexSizeMap();

  // Index by lat long.
  for (const shared_ptr<tNeighborhoodInfoWithBounds>& nbr : storage()) {
    // don't add to index if there are no hotels in it.
    // TODO(pramodg, akearney): Explain the zoom level check.
    if (nbr->loc.hotel_count == 0 || nbr->loc.zoom_level < 3) continue;

    LatLong ll = LatLong::Create(nbr->lat, nbr->lon);
    int zoom_grid_size = ::util::tl::FindWithDefault(zoom_indexsize_map_, nbr->loc.zoom_level, 0);
    ASSERT_GT(zoom_grid_size, 0) << "Invalid zoom index level for: " << nbr->ToJSON({1, 1});
    neighborhood_latlong_indices_[nbr->loc.zoom_level].AddToIndex(pair<int, int>(
        static_cast<int>(ll.get_multiplied_latitude() / zoom_grid_size),
        static_cast<int>(ll.get_multiplied_longitude() / zoom_grid_size)),
        nbr.get(), false);
  }
}

void Neighborhoods::BuildIdToLocationMap() {
  for (const shared_ptr<tNeighborhoodInfoWithBounds>& nbr : storage()) {
    if (nbr->loc.zoom_level < 3) continue;  // TODO(pramodg, akearney): Why is this check here ?
    id_location_map_.emplace(nbr->loc.id, nbr.get());
  }
}

void Neighborhoods::BuildZoomToIndexSizeMap() {
  if (!gFlag_maponics_index_file.empty()) {
    ifstream file(gFlag_maponics_index_file.c_str());
    ASSERT(file.good()) << "File not found: " << gFlag_maponics_index_file;
    while (!file.fail()) {
      string line;
      getline(file, line);
      if (file.fail()) {
        LOG(ERROR) << "Failed to read line.";
        break;
      }
      istringstream iss(line);
      int zoom_level = -1;
      float grid_size = 0;
      iss >> zoom_level >> grid_size;
      if (zoom_level == -1 || grid_size == 0) {
        LOG(ERROR) << "Could not parse line: " << line;
        break;
      }

      // TODO(akearney): Please add rationale for (20000000.0/3.0).
      zoom_indexsize_map_.emplace(zoom_level, static_cast<int>(20000000.0 / 3.0 * grid_size));
    }
  }
  LOG(INFO) << "Init Zoom map of size: " << zoom_indexsize_map_.size();
}

void Neighborhoods::BuildHidToLocationMap() {
  if (!gFlag_hotel_to_location_file.empty()) {
    LOG(INFO) << "Loading file: " << gFlag_hotel_to_location_file;
    ifstream f(gFlag_hotel_to_location_file.c_str());
    ASSERT(f.good()) << "File not found: " << gFlag_hotel_to_location_file;

    int seq_hid = -1;
    // For this we assume that in the file all hid to nid in the file
    // are sequential, which is fair since this is how it's written
    // in the backend
    while (!f.fail()) {
      string line;
      getline(f, line);
      if (f.fail()) break;
      istringstream iss(line);
      int hid = -1;
      string nid;
      iss >> hid >> nid;
      if (hid == -1 || nid.empty()) {
        LOG(ERROR) << "Could not parse line: " << line;
        break;
      }

      // Check if this is a new hid.
      if (hid != seq_hid) {
        seq_hid = hid;
        hid_location_range_map_[hid] = {location_list_.size(), 0};
      }

      // Check if the neighborhood is valid.
      const tNeighborhoodInfoWithBounds* nbr = LookupUniqueByNeighborhoodId(nid);
      if (nbr != nullptr) {
        location_list_.push_back(nbr);
        ++(hid_location_range_map_[hid].second);
      }
    }
  }
  LOG(INFO) << "Built hid to location map for " << hid_location_range_map_.size() << " hotels.";
}

vector<const tNeighborhoodInfoWithBounds*> Neighborhoods::LookupByHid(int hid) const {
  vector<const tNeighborhoodInfoWithBounds*> res;
  static const pair<int, int> kDefault(-1, 0);
  pair<int, int> p = ::util::tl::FindWithDefault(hid_location_range_map_, hid, kDefault);

  // Validate indices.
  if (p.first < 0 || (p.first + p.second) > location_list_.size()) return res;

  res.reserve(p.second);
  for (int i = 0; i < p.second; ++i)
    res.push_back(location_list_[p.first + i]);

  return res;
}

// Regions are generously contained by a 3x3 grid for each level. This means in terms
// of longs the grid size is by zoom_level to approximate miles :
// This data as of 8/21/12 but could change if the maponics data changes.
// for reference, hotels use 7,000,000
// zoom level : latlong grid size : approx nautical miles (at equator)
//
// 0 : 19,711,867 : 1182
// 1 : 14,294,200 :  858
// 2 :  9,921,411 :  594
// 3 :    878,307 :   53
// 4 :  1,149,793 :   69
// 5 :    195,795 :   12
// 6 :     55,309 :    3
//
int Neighborhoods::LookupByLatLong(const LatLong& ref_point,
     map<int, vector<pair<const tNeighborhoodInfoWithBounds*, double>>>* result,
     int max_results, float max_radius_km, int min_hotel_in_neighborhood) const {
  result->clear();
  int count = 0;
  if (max_results == 0 || max_radius_km == 0) return 0;
  // Iterate over all zoom levels and create a separate result set for each level.
  for (const pair<int, int>& zoom_size : zoom_indexsize_map_) {
    auto index_it = neighborhood_latlong_indices_.find(zoom_size.first);
    if (index_it == neighborhood_latlong_indices_.end()) continue;
    const auto& latlong_index = index_it->second;

    // Get the lat long grid id.
    int latitude_grid_id =
        static_cast<int>(ref_point.get_multiplied_latitude() / zoom_size.second);
    int longitude_grid_id =
        static_cast<int>(ref_point.get_multiplied_longitude() / zoom_size.second);

    vector<pair<const tNeighborhoodInfoWithBounds*, double>> zoom_res;
    // retrieve from 9 nearby grid cells
    for (int i = -1; i < 2; ++i) {
      for (int j = -1; j < 2; ++j) {
        vector<const tNeighborhoodInfoWithBounds*> nearby;
        latlong_index.Retrieve(pair<int, int>(latitude_grid_id + i, longitude_grid_id + j),
                               &nearby);
        for (const tNeighborhoodInfoWithBounds* nbr : nearby) {
          // Check if the nbr has enough hotels.
          if (nbr->loc.hotel_count < min_hotel_in_neighborhood) continue;

          LatLong ll = LatLong::Create(nbr->lat, nbr->lon);
          // Check if the lat long distance is within the accepted range.
          if (abs(ll.get_multiplied_latitude() - ref_point.get_multiplied_latitude())
              > zoom_size.second ||
              abs(ll.get_multiplied_longitude() - ref_point.get_multiplied_longitude())
              > zoom_size.second) continue;
          // now calculate accurate surface distance.
          float distance = ref_point.SurfaceDistanceFrom(ll);
          if (max_radius_km > 0 && distance > max_radius_km) continue;
          zoom_res.emplace_back(nbr, distance);
        }
      }
    }

    // Sort the matches by distance.
    if (zoom_res.size()) {
      partial_sort(zoom_res.begin(),
                   zoom_res.begin() + min<int>(max_results, zoom_res.size()),
                   zoom_res.end(),
                   ::util::tl::less_second<pair<const tNeighborhoodInfoWithBounds*, double>>());

      vector<const tNeighborhoodInfoWithBounds*> dup_candidates;
      dup_candidates.reserve(zoom_res.size());
      for (const auto& pair : zoom_res) dup_candidates.push_back(pair.first);
      // Dedup the results.
      Dedup(&dup_candidates);
      if (dup_candidates.size() < zoom_res.size()) {
        int i = 0;
        auto iter = remove_if(zoom_res.begin(), zoom_res.end(),
            [&dup_candidates, &i](const pair<const tNeighborhoodInfoWithBounds*,
                double>& pair) -> bool {
              if (i < dup_candidates.size() && dup_candidates[i] == pair.first) {
                ++i;
                return false;
              }
              return true;
            });
        if (iter != zoom_res.end()) zoom_res.resize(iter - zoom_res.begin());
      }

      if (zoom_res.size() > max_results) zoom_res.resize(max_results);
      count += zoom_res.size();
      result->emplace(zoom_size.first, zoom_res);
    }
  }
  return count;
}

void Neighborhoods::RetrieveNeighborhood(const string& name, const string& mcd,
    const string& state_code, vector<const tNeighborhoodInfoWithBounds*>* result,
    int max_results) const {
  // Try the complete name first.
  vector<string> name_parts = {name, mcd, state_code};
  LookupByName(strutil::JoinString(name_parts, " "), result, max_results);
  if (result->size() > 0) return;

  // Try the name without state.
  name_parts = {name, mcd};
  LookupByName(strutil::JoinString(name_parts, " "), result, max_results);
  if (result->size() > 0) return;

  // Try the name without mcd.
  name_parts = {name, state_code};
  LookupByName(strutil::JoinString(name_parts, " "), result, max_results);
  if (result->size() > 0) return;

  // Try the name without mcd and state.
  name_parts = {name};
  LookupByName(strutil::JoinString(name_parts, " "), result, max_results);
}

void Neighborhoods::RetrieveNeighborhood(const string& name, const string& mcd,
    const string& state_code, vector<tNeighborhoodInfoReply>* result,
    int max_results) const {
  vector<const tNeighborhoodInfoWithBounds*> nbrs;
  RetrieveNeighborhood(name, mcd, state_code, &nbrs, max_results);
  result->clear();
  if (nbrs.empty()) return;

  result->reserve(nbrs.size());
  neighborhood_storage_to_reply conv;
  for (const tNeighborhoodInfoWithBounds* nbr : nbrs)
    result->emplace_back(conv(nbr));
}

// Register the countries with the enity manager.
auto reg_em_neighborhoods = ::entity::EntityManager::bind("em_neighborhoods", "",
    InitializeConfigureConstructor<Neighborhoods, string>());

}  // namespace region_data

// Init before code translator.
INIT_ADD("neighborhoods", 0, []{ region_data::Neighborhoods::Instance(); });
