// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/suggest/common/twiddler/util/suggest_twiddler_utils.h"

namespace suggest {
namespace twiddle {

// Updates the suggest response completions with the twiddler response.
bool UpdateCompletionsWithTwiddlerResponse(
    const SuggestRequest& suggest_request,
    const SuggestTwiddleResponse& twiddler_response,
    SuggestResponse* suggest_response) {
  if (twiddler_response.completion_scores.size() !=
      suggest_response->completions.size()) {
    ASSERT_DEV_EQ(twiddler_response.completion_scores.size(),
        suggest_response->completions.size()) << "Mismatch in size of "
            << "suggestions and twiddler response scores.";
    return false;
  }

  for (int i = 0; i < twiddler_response.completion_scores.size(); ++i) {
    Completion& completion = suggest_response->completions[i];
    const SuggestTwiddleResponse::CompletionScore& twiddler_score =
        twiddler_response.completion_scores[i];

    // Multiply the socre with the twiddler to get the boosted score.
    completion.score *= twiddler_score.score;

    completion.debug_info.append(" | Twiddler : " +
                                 std::to_string(twiddler_score.score) +
                                 " (" + twiddler_score.debug_info + ")");
  }
  return true;
}

}  // namespace twiddle
}  // namespace suggest
