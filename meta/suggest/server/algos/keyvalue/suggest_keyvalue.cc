// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include <fstream>
#include "meta/suggest/server/algos/keyvalue/suggest_keyvalue.h"

namespace suggest {
namespace algo {

namespace {

// Initializes the key value map from file.
template<typename T>
bool InitKVMapFromFile(const string& file, T* kv_map) {
  ifstream f(file.c_str());
  if (!f.good()) {
    LOG(ERROR) << "File not found: " << file;
    return false;
  }

  if (!serial::Serializer::FromBinary(f, kv_map)) {
    LOG(ERROR) << "Could not parse file: " << file;
    return false;
  }
  return true;
}

}  // namespace

// Initialize the class.
bool SuggestKeyValue::Initialize() {
  if (!super::Initialize()) return false;
  kv_map_.reset(new SuggestKeyValueMap);

  if (!InitKVMapFromFile(params().file, kv_map_.get())) return false;

  LOG(INFO) << "Successfully Initialized " << params().name;
  return true;
}

int SuggestKeyValue::FindCompletions(const SuggestRequest& request,
    shared_ptr<SuggestResponse> response,
    shared_ptr<SuggestAlgoContext> context) const {
  const auto iter = kv_map_->find(request.normalized_query);

  if (iter == kv_map_->end()) return 0;

  response->completions.reserve(response->completions.size() +
                                iter->second.size());
  for (const CompletionIndexItem& item : iter->second) {
    Completion completion(item);
    completion.algo_type = params().type;

    // Add the completion to the response.
    response->completions.push_back(completion);
  }
  return iter->second.size();
}

// Initialize the class.
bool SuggestKeyValueEx::Initialize() {
  if (!super::Initialize()) return false;
  kv_ex_map_.reset(new SuggestKeyValueExMap);

  if (!InitKVMapFromFile(params().file, kv_ex_map_.get())) return false;

  LOG(INFO) << "Successfully Initialized " << params().name;
  return true;
}

int SuggestKeyValueEx::FindCompletions(const SuggestRequest& request,
    shared_ptr<SuggestResponse> response,
    shared_ptr<SuggestAlgoContext> context) const {
  const auto iter = kv_ex_map_->find(request.normalized_query);

  if (iter == kv_ex_map_->end()) return 0;

  response->completions.reserve(response->completions.size() +
                                iter->second.size());
  for (const CompletionIndexItemEx& item : iter->second) {
    Completion completion(item);
    completion.algo_type = params().type;

    // Check if the item has an index score.
    if (completion.index_score) completion.score = completion.index_score;

    // Add the completion to the response.
    response->completions.push_back(completion);
  }
  return iter->second.size();
}

}  // namespace algo
}  // namespace suggest
