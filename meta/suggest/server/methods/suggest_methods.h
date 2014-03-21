// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _META_SUGGEST_SERVER_METHODS_SUGGEST_METHODS_H_
#define _META_SUGGEST_SERVER_METHODS_SUGGEST_METHODS_H_

#include "base/common.h"
#include "meta/suggest/common/suggest_datatypes.h"
#include "util/network/method/server_method.h"
#include "util/serial/serializer.h"

namespace suggest {
namespace methods {

class GetSuggestions : public network::ServerMethod {
 public:
  struct ReleaseReply {
    struct CompleteSuggestionReply : public CompleteSuggestion {
      CompleteSuggestionReply() = default;
      CompleteSuggestionReply(const CompleteSuggestionReply&) = default;
      CompleteSuggestionReply(const CompleteSuggestion& c)
          : CompleteSuggestion(c) {}

      // This is the aggregated string from all annotations collected for the
      // suggestion.
      string annotation;

      // Set to true if the suggestion is a child suggestion.
      bool child = false;

      // This is the string that is filled in case the display string is not
      // the same as the query string that should be filled in the text box.
      // This is required for the child suggestions where the query string
      // and the display string differ significantly.
      string query;

      SERIALIZE(DEFAULT_CUSTOM / src_type*1 / src_id*2 / latitude*5 /
                longitude*6 / display*8 / annotation*11 / child*12 / query*13);
    };

    bool success = false;
    vector<CompleteSuggestionReply> suggestions;

    // Set to true if the first suggestion is instant search worthy.
    bool enable_instant = false;

    SERIALIZE(DEFAULT_CUSTOM / success*1 / suggestions*2 / enable_instant*3);
  };

  struct DebugReply : public SuggestResponse {
    DebugReply() = default;
    DebugReply(const DebugReply&) = default;
    DebugReply(const SuggestResponse& r) : SuggestResponse(r) {}

    SERIALIZE(DEFAULT_CUSTOM / success*1 / completions*2);
  };

  string operator()(const SuggestQuery& req, ReleaseReply* result) const;

  string operator()(const SuggestQuery& req, DebugReply* result) const;

  static SuggestQuery ExampleRequest() {
    SuggestQuery query;
    query.input = "san fr";
    return query;
  }

 private:
  SuggestRequestInterface PrepareRequest(const SuggestQuery& req) const;

};

}  // namespace methods
}  // namespace suggest

#endif  // _META_SUGGEST_SERVER_METHODS_SUGGEST_METHODS_H_
