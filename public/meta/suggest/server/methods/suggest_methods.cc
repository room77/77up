// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramd Gupta)


#include "meta/suggest/server/methods/suggest_methods.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

#include "util/string/strutil.h"
#include "meta/suggest/server/methods/suggest_method_utils.h"
#include "meta/suggest/server/suggestions/suggestion_manager.h"
#include "util/log/channel/channel.h"
#include "util/entity/entity_id.h"
#include "util/templates/container_util.h"

namespace suggest {
namespace methods {
namespace {

string GetDisambiguationNameKeyFromSuggestion(const CompleteSuggestion& suggestion) {
  string name_key = suggestion.normalized;
  if (suggestion.annotations.size() && !suggestion.annotations.back().empty())
    name_key += " " + suggestion.annotations.back();

  return name_key;
}

}  // namespace

string GetSuggestions::operator()(const SuggestQuery& req,
                                  GetSuggestions::DebugReply* result) const {
  SuggestRequestInterface suggest_request = PrepareRequest(req);
  suggest_request.debug = true;

  shared_ptr<SuggestResponse> suggest_response(new SuggestResponse);
  SuggestionManager::Instance()->GetCompletions(suggest_request,
                                                suggest_response);

  *result = *suggest_response;
  return (result->success) ? "" : "Request Failed";
}

string GetSuggestions::operator()(const SuggestQuery& req,
                                  GetSuggestions::ReleaseReply* result) const {
  SuggestRequestInterface suggest_request = PrepareRequest(req);
  suggest_request.debug = false;

  shared_ptr<SuggestResponse> suggest_response(new SuggestResponse);
  SuggestionManager::Instance()->GetCompletions(suggest_request,
                                                suggest_response);

  // Copy over data from suggest response.
  result->success = suggest_response->success;
  if (!result->success) return "Request Failed";

  result->enable_instant = suggest_response->enable_instant;

  result->suggestions.reserve(suggest_response->completions.size());

  // Fill the parent id to srcid mapping.
  unordered_map<SuggestionId, std::reference_wrapper<const Completion>> sgstid_completion_map;
  unordered_map<string, int> disambiguation_name_count_map;
  for (const Completion& completion : suggest_response->completions) {
    if (!completion.parent_id.empty()) continue;
    sgstid_completion_map.insert(make_pair(completion.suggestion_id, std::cref(completion)));

    if (completion.suggestion->src_type == entity::kEntityTypeCity) {
      const string name_key = GetDisambiguationNameKeyFromSuggestion(*completion.suggestion);
      disambiguation_name_count_map[name_key] += 1;
    }
  }

  for (const Completion& completion : suggest_response->completions) {
    ReleaseReply::CompleteSuggestionReply reply(*completion.suggestion);

    if (completion.parent_id.size()) {
      // Fix child.
      FixChildSuggestion(completion,
          ::util::tl::FindOrDie(sgstid_completion_map, completion.parent_id).second, &reply);
    } else {
      // Fix normal/parent suggestion.
      FixParentSuggestion(suggest_request, ::util::tl::FindWithDefault(
          disambiguation_name_count_map, GetDisambiguationNameKeyFromSuggestion(
              *completion.suggestion), 1), &reply);
    }

    // Add the suggestion to the reply.
    result->suggestions.push_back(reply);
  }

  return "";
}

SuggestRequestInterface GetSuggestions::PrepareRequest(
    const SuggestQuery& req) const {
  SuggestRequestInterface suggest_request(req);

  suggest_request.user_country = GetUserCountry();
  suggest_request.device_channel = ::meta::channel::GetChannel(arg_map());
  suggest_request.is_mobile =
      ::meta::channel::IsMobile(suggest_request.device_channel);

  return suggest_request;
}

network::ServerMethodRegister<SuggestQuery, GetSuggestions::ReleaseReply,
    GetSuggestions> reg_Suggestions("suggest_server", "GetSuggestions",
                                    GetSuggestions::ExampleRequest());

network::ServerMethodRegister<SuggestQuery, GetSuggestions::DebugReply,
    GetSuggestions> reg_DebugSuggestions("suggest_server",
                                         "GetDebugSuggestions",
                                         GetSuggestions::ExampleRequest());

}  // namespace methods
}  // namespace suggest
