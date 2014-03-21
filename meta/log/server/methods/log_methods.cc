// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "base/defs.h"
#include "util/init/init.h"
#include "meta/log/common/log_datatypes.h"
#include "meta/log/common/manager/log_manager.h"
#include "util/log/channel/channel.h"
#include "util/network/method/server_method.h"
#include "util/serial/types/arbit_blob.h"
#include "util/time/timestamp.h"

namespace logging {
namespace methods {

class Log: public network::ServerMethod {
 public:
  string operator()(const tLogRawRequest& req, string* result) const {
    shared_ptr<tLogRequestInterface> log_request(new tLogRequestInterface);

    // populate the request with the data from the header
    log_request->user_id = GetUserId();
    log_request->session_id = GetSessionId();

    // created is not a good name, but for now we are double-logging the variable because of
    // consistency with the old logs...received_time is what should be used from now on
    log_request->received_time = log_request->created =
        ::util::Timestamp::Now<chrono::microseconds>();

    log_request->url = req.url;
    log_request->cgi_params = req.cgi_params;
    log_request->user_ip = GetUserIP();

    // agent is not a good name, but for now we are double-logging the variable because of
    // consistency with the old logs...user_agent is what should be used from now on
    log_request->user_agent = log_request->agent = GetHttpHeader("User-Agent");

    log_request->lang = GetHttpHeader("Accept-Language");
    log_request->host = GetHttpHeader("Host");
    log_request->channel = meta::channel::GetChannelString(arg_map());
    log_request->is_mobile =
        meta::channel::IsMobile(meta::channel::GetChannel(log_request->channel));
    log_request->user_country = GetUserCountry();

    LogManager::Instance().ProcessLogsAsync(req.logs, log_request, req.sent_time);

    return "";
  }

  // function to create a mockup request
  static tLogRawRequest ExampleRequest() {
    tLogRawRequest req;
    req.sent_time = 1377538518894370;
    req.url = "url";
    req.logs.push_back(
        "{\n"
        "  \"category\":\"Hotel Search\",\n"
        "  \"action\":\"Search Exps\",\n"
        "  \"value\": {\n"
        "    \"city\":\"IE\",\n"
        "    \"ctrseg\":\"IE\",\n"
        "    \"dist\":\"IA\",\n"
        "    \"mic\":\"A\",\n"
        "    \"session\":\"IE\",\n"
        "    \"hid\":0,\n"
        "  },\n"
        "  \"nid\":\"\",\n"
        "  \"pid\":\"\",\n"
        "  \"id\":\"4RFCmZ\",\n"
        "  \"user_time\":1378923081011676,\n"
        "}\n");

    req.logs.push_back(
        "{\n"
        "  \"category\":\"New Visit\",\n"
        "  \"action\":\"Referrer\",\n"
        "  \"value\": {\n"
        "    \"value\":\"\",\n"
        "  },\n"
        "  \"nid\":\"\",\n"
        "  \"pid\":\"\",\n"
        "  \"id\":\"4RFCmZ\",\n"
        "  \"user_time\":1378923081011748,\n"
        "}\n");

    return req;
  }
};

INIT_ADD_REQUIRED("log_method", [] {
  network::ServerMethodRegister<tLogRawRequest, string, Log> reg_Log("log_server", "Log",
      Log::ExampleRequest());
});

}  // namespace methods
}  // namespace logging

