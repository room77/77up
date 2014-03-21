// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/server/methods/suggest_method_utils.h"

#include <functional>
#include <unordered_set>

#include "util/string/strutil.h"
#include "util/file/csvreader.h"
#include "util/entity/entity_id.h"
#include "util/region_data/countries.h"
#include "util/region_data/states.h"
#include "util/templates/container_util.h"
#include "util/templates/equator.h"
#include "util/templates/hash.h"

FLAG_string(suggest_child_display_name_modifier_file,
            "static_data/push/auto/suggest/display_name_modifier",
            "The display name modifier file.");

FLAG_string(suggest_child_display_name_modifier_region_custom_eid, "m/region",
            "The custom entity id used to get the child display name modifier for region based "
            "entity ids.");

FLAG_string(suggest_child_display_name_modifier_neighborhood_custom_eid,
            "m/neighborhood",
            "The custom entity id used to get the child display name modifier"
              " for neighborhood entity ids.");

namespace {

struct tChildDisplayModifier {
  explicit tChildDisplayModifier(const string& id = "") : eid(id) {}
  string eid;
  string modifier;
  bool before = true;
  CSV(eid | modifier | before);
};

string ChildDisplayname(const string* eid, const string& display_name) {
  // Hasher and equator for entity id.
  typedef ::util::tl::hash_member<tChildDisplayModifier, string,
      &tChildDisplayModifier::eid> ChildDisplayModifierIdHasher;
  typedef ::util::tl::equal_member<tChildDisplayModifier,
      string, &tChildDisplayModifier::eid> ChildDisplayModifierIdEquator;
  // Map (implemented as set) from entity id -> tChildDisplayModifier.
  typedef unordered_set<tChildDisplayModifier, ChildDisplayModifierIdHasher,
      ChildDisplayModifierIdEquator> ModifierIdMap;

  struct Creator {
    ModifierIdMap CreateModifierMap() {
      ModifierIdMap id_map;
      CSV::CSVReader<ModifierIdMap> reader(gFlag_suggest_child_display_name_modifier_file, '|');
      reader.Read(&id_map);
      return id_map;
    }

    tChildDisplayModifier DefaultModifier(const ModifierIdMap & modifier_map) {
      tChildDisplayModifier res("*");
      res = ::util::tl::FindWithDefault(modifier_map, res, res);
      return res;
    }
  };

  static const ModifierIdMap modifier_id_map = Creator().CreateModifierMap();
  static const tChildDisplayModifier kDefaultModifier = Creator().DefaultModifier(modifier_id_map);

  // NOTE modifier rules for specific entity types should override the rules
  // for generic entity types.
  // e.g. the neighborhood type modifier rule overrides the rule for
  // generic region-based entity types.
  const auto entity_type = ::entity::GetEntityTypeFromEntityId(*eid);
  if (::entity::IsTypeNeighborhood(entity_type)) {
    eid = &gFlag_suggest_child_display_name_modifier_neighborhood_custom_eid;
  } else if (::entity::IsTypeRegion(entity_type)) {
    eid = &gFlag_suggest_child_display_name_modifier_region_custom_eid;
  }

  const tChildDisplayModifier& modifier =
      ::util::tl::FindWithDefault(modifier_id_map, tChildDisplayModifier(*eid), kDefaultModifier);

  return (modifier.before) ? modifier.modifier + display_name : display_name + modifier.modifier;
}

// Formats the annotations for the parent suggestion.
string FormatAnnotations(const suggest::SuggestRequestInterface& req, int name_count,
                         const vector<string>& annotations) {
  string res = (annotations.size() > 2) ?
      strutil::JoinString(annotations.begin(), annotations.end() - 2) : "";

  // The last annotation is country.
  string country_code, country_name;
  if (annotations.size() && !annotations.back().empty()) {
    country_code = annotations.back();
    // If this is a foreign country for the user, we want to get country name.
    if (country_code != req.user_country)
      country_name = region_data::Countries::Instance().GetCountryName(country_code);
  }

  // The second last annotation is state.
  string state_code, state_name;
  if (annotations.size() > 1 && !country_code.empty()) {
    state_code = *(annotations.end() - 2);
    // If this is a foreign country for the user, we want to get state name.
    if (!state_code.empty() && (!country_name.empty() || strutil::IsNumeric(state_code))) {
      state_name = region_data::States::Instance().GetStateName(state_code, country_code);
      // We do not want to display this even if state name is empty.
      state_code.clear();
    }
  }

  vector<std::reference_wrapper<const string>> parts;
  if (!res.empty()) parts.push_back(std::cref(res));

  // Display the state code (name) only when:
  // 1. If the user is in the same country.
  // 2. If the city name exists multiple times in the suggestions list.
  if (country_code == req.user_country || name_count > 1) {
    if (!state_name.empty()) parts.push_back(std::cref(state_name));
    else if (!state_code.empty()) parts.push_back(std::cref(state_code));
  }

  if (!country_name.empty()) parts.push_back(std::cref(country_name));
  else if (!country_code.empty() && country_code != req.user_country)
    parts.push_back(std::cref(country_code));

  return strutil::JoinString(parts);
}

}  // namespace

namespace suggest {
namespace methods {

void FixParentSuggestion(const SuggestRequestInterface& req, int name_count,
                         GetSuggestions::ReleaseReply::CompleteSuggestionReply* reply) {
  // Fix annotations.
  reply->annotation = FormatAnnotations(req, name_count, reply->annotations);

  // Fix query.
  reply->query = strutil::JoinTwoStrings(reply->display, reply->annotation, ", ");
}

void FixChildSuggestion(const Completion& child, const Completion& parent,
                        GetSuggestions::ReleaseReply::CompleteSuggestionReply* reply) {
  reply->child = true;
  const string orig_src_id = reply->src_id;
  reply->src_id = child.suggestion_id;

  // Fix query. The query for children looks like
  // [parent_display child_display].
  reply->query = strutil::JoinTwoStrings(parent.suggestion->display,
                                               reply->display, " ");

  // TODO(pramodg): We need a more generic solution for display prefix.
  reply->display = ChildDisplayname(&orig_src_id, reply->display);

  // Add parent lat longs if the child does not have them.
  if (reply->latitude == 0 && reply->longitude == 0) {
    reply->latitude = parent.suggestion->latitude;
    reply->longitude = parent.suggestion->longitude;
  }
}

}  // namespace methods
}  // namespace suggest
