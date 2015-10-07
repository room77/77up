// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "base/args/args.h"
#include "util/network/method/server_method.h"

namespace network {
namespace {

class EditParam : public network::ServerMethod {
 public:
  struct Request {
    string name, value;
    bool restore = false;
    SERIALIZE(DEFAULT_CUSTOM / name*1 / value*2 / restore*3);
  };

  struct Result {
    string message, value;
    bool restored = false;
    SERIALIZE(message*1 / value*2 / restored*3);
  };

  string operator()(const Request& req, Result* reply) const {
    Result result;

    args::CommandLineArgs& c = args::CommandLineArgs::Instance();

    string err;
    if (req.restore) err = c.RestoreArgumentValue(req.name);
    else err =  c.AssignArgument(req.name, req.value, true);

    if (err.empty()) {
      reply->value = c.GetArgumentValue(req.name);
      reply->message = "Successfully updated: " + req.name + " to " +
          reply->value + ".";
      reply->restored = req.restore;
    } else {
      err = "Unable to update parameter: " + err;
    }
    return err;
  }

  static Request ExampleRequest() {
    Request req;;
    req.name = "loglevel";
    req.value = "2";
    req.restore = false;
    return req;
  }
};

ServerMethodRegister<EditParam::Request, EditParam::Result, EditParam>
    reg_EditParam("all", ".edit_param", EditParam::ExampleRequest());

}  // namespace
}  // namespace network
