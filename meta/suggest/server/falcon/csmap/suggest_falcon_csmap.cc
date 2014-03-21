// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "meta/suggest/server/falcon/csmap/suggest_falcon_csmap.h"

#include <fstream>
#include "util/serial/serializer.h"

namespace suggest {
namespace falcon {

// Initialize the class.
bool SuggestFalconCSMap::Initialize() {
  LOG(INFO) << "Initializing falcon suggestions from file: " << params().file;

  if (params().file.empty()) {
    LOG(ERROR) << "No file specified!";
    return false;
  }

  ifstream f(params().file.c_str());
  if (!f.good()) {
    LOG(ERROR) << "File not found: " << params().file;
    return false;
  }

  if (!serial::Serializer::FromBinary(f, &suggestion_map_)) {
    LOG(ERROR) << "Could not parse file: " << params().file;
    return false;
  }

  return true;
}

// Fills the response with the pointers to complete suggestions.
void SuggestFalconCSMap::AddCompleteSuggestions(
    shared_ptr<SuggestResponse> response,
    shared_ptr< ::util::threading::Counter> counter) const {
  ::util::threading::ScopedCNotify n(counter.get());
  if (response == nullptr) return;

  for (Completion& completion : response->completions) {
    if (completion.suggestion != nullptr) continue;

    completion.suggestion = Find(completion.suggestion_id);
    if (completion.suggestion != nullptr) {
      // Set the default score as the base score for the completion.
      if (completion.score == 0)
        completion.score = completion.suggestion->base_score;
    } else {
      ASSERT_DEV(false) << " Could not find suggestion for id: "
                        << completion.suggestion_id << ", Falcon size: "
                        << size();
    }
  }

  // Remove completions with no suggestions.
  auto iter = remove_if(response->completions.begin(),
                        response->completions.end(),
                        [](const Completion& completion) -> bool {
                             if (completion.suggestion == nullptr) return true;
                             return false;
                        });

  if (iter != response->completions.end())
    response->completions.resize(iter - response->completions.begin());
}

}  // namespace falcon
}  // namespace suggest
