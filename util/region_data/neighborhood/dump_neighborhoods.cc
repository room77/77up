#include "util/region_data/neighborhood/neighborhoods.h"

#include "base/defs.h"

FLAG_string(neighborhoods_dump_file,
            "/tmp/neighborhoods.json",
            "Location of neighborhoods json dump.");

namespace region_data { namespace {

struct tNeigh {
  tNeigh() = default;
  tNeigh(const tNeighborhoodInfoWithBounds& n) {
    const auto& loc = n.loc;
    id = loc.id;
    name = loc.name;
    state = loc.state;
    mcd = loc.mcd;
    zoom_level = loc.zoom_level;

    parents = serial::Serializer::ToJSON(loc.parents, -1);
    center = serial::Serializer::ToJSON(loc.center, -1);
    children = serial::Serializer::ToJSON(loc.children, -1);
  }

  string id;
  string name;
  string state;
  string mcd;
  int zoom_level = -1;

  string parents;
  string center;
  string children;

  CSV(id | name | state | mcd | zoom_level | parents | center | children);
  SERIALIZE(id*1 / name*2 / state*3 / mcd*4 / zoom_level*5 / parents*6 /
            center*7 / children*8);
};

void DumpJson(const string& file_name) {
  vector<const tNeighborhoodInfoWithBounds *> neighs;
  neighs.reserve(Neighborhoods::Instance().storage().size());
  for (const auto& sp_neigh : Neighborhoods::Instance().storage()) {
    neighs.emplace_back(sp_neigh.get());
  }

  ofstream dump_output(file_name);
  ASSERT(dump_output.good());
  dump_output << serial::Serializer::ToJSON(neighs, {1, 1});
  dump_output.close();
}

void DumpCsv(const string& file_name) {
  ofstream dump_output(file_name);
  ASSERT(dump_output.good());

  dump_output << tNeigh().ToCSV(',', true) << "\n";

  for (const auto& sp_neigh : Neighborhoods::Instance().storage()) {
    dump_output << tNeigh(*sp_neigh).ToCSV(',', false) << "\n";
  }
  dump_output.close();
}

void Investigate(const string& id) {
  unordered_map<string, shared_ptr<tNeighborhoodInfoWithBounds>> m;
  for (const auto& sp_neigh : Neighborhoods::Instance().storage()) {
    const auto insert_ok = m.emplace(sp_neigh->loc.id, sp_neigh).second;
    ASSERT(insert_ok);
  }

  const auto& the_neigh = *m.find(id)->second;
  for (const auto& parent_id : the_neigh.loc.parents) {
    const auto& parent = *m.find(parent_id)->second;
    if (parent.loc.zoom_level == 0) continue;
    LOG(INFO) << tNeigh(parent).ToJSON(-1);
  }
}

}  // namespace unnamed

}  // namespace region_data

int init_main() {
  region_data::DumpCsv(gFlag_neighborhoods_dump_file);

//  region_data::Investigate("7700");

  return 0;
}
