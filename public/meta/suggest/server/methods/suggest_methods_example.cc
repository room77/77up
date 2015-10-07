#include "util/init/main.h"
#include "meta/suggest/server/methods/suggest_methods.h"

namespace suggest { namespace methods {

void RunExample() {
  SuggestQuery req;
  GetSuggestions::ReleaseReply result;

  req.input = "san fr";

  const auto message = GetSuggestions()(req, &result);

  LOG(INFO) << result.ToJSON({1, 1});

  ASSERT(message.empty());
}

}  // namespace methods
}  // namespace suggest

int init_main() {
  suggest::methods::RunExample();
  return 0;
}
