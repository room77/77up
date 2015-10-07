// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: otasevic@room77.com (Nikola Otasevic)

#include "base/args/args.h"
#include "util/network/method/server_method.h"
#include "util/counter/counter_base.h"

namespace network {
namespace {

struct tEventReq {
  string counter_name;
  uint64_t interval = 0;  // how far back (in microseconds)
  vector<string> metrics;
  SERIALIZE(DEFAULT_CUSTOM / counter_name*1 / interval*2 / metrics*3);
};

class GetCounters : public network::ServerMethod {
 public:

  typedef vector<tEventReq> Request;
  struct tEventReply {
    string message = "ok";
    vector<float> metric_results;
    SERIALIZE(DEFAULT_CUSTOM / metric_results*1);
  };
  typedef vector<tEventReply> Reply;

  string operator()(const Request& req, Reply* reply) const {
    vector<string> all_events = counter::CounterBase::keys<vector>();

    for (const tEventReq& event_req : req) {
      tEventReply event_reply;
      if (find(all_events.begin(), all_events.end(), event_req.counter_name) != all_events.end()) {
        counter::CounterBase::mutable_shared_proxy counter =
            counter::CounterBase::make_shared(event_req.counter_name);
        counter::CounterBase::MetricsMap m = counter->GetMetricsForInterval(event_req.interval,
                                                                            event_req.metrics);
        for (const string& metric_name : event_req.metrics) {
          counter::CounterBase::MetricsMap::iterator it = m.find(metric_name);
          if (it != m.end()) {
            event_reply.metric_results.push_back(static_cast<float>(*it->second));
          } else {
            event_reply.metric_results.push_back(0);
          }
        }
      } else {
        event_reply.message = "error";
        event_reply.metric_results.assign(event_req.metrics.size(), 0);
      }
      reply->push_back(event_reply);
    }

    return "";
  }

  static Request ExampleRequest() {
    Request req;
    tEventReq event_req1;
    event_req1.counter_name = "NewVisitCounter";
    event_req1.interval = 60 * 1000 * 1000ULL; // last 1 minute
    event_req1.metrics = { "count", "sum", "mean"};
    tEventReq event_req2;
    event_req2.counter_name = "HotelProfileClickCounter";
    event_req2.interval = 60 * 1000 * 1000ULL; // last 1 minute
    event_req2.metrics = { "count", "sum", "mean"};
    req.push_back(event_req1);
    req.push_back(event_req2);
    return req;
  }
};

// simple counter method for external querying of single value. this method
// is used by zabbix to compute stats and allows override by cgi arguments:
//   name = overrides the tEventReq.counter_name
//   secs  = the number of seconds
//   type = the type of metric requested, e.g. "count", "sum", "mean"
class GetCounter : public network::ServerMethod {
 public:
  typedef tEventReq Request;
  typedef float Reply;

  string operator()(const Request& req, Reply* reply) const {
    GetCounters::Request counters_req;
    counters_req.push_back(req);
    // override with the cgi arguments if they exist
    auto& counter_req = *counters_req.rbegin();
    if (arg_map().find("name") != arg_map().end())
      counter_req.counter_name = arg_map().find("name")->second;
    if (arg_map().find("type") != arg_map().end())
      counter_req.metrics = { arg_map().find("type")->second };
    if (arg_map().find("secs") != arg_map().end()) {
      float nsecs;
      istringstream(arg_map().find("secs")->second) >> nsecs;
      counter_req.interval = 1e6 * nsecs;
    }
    VLOG(4) << counter_req.ToJSON();
    // cascade out
    GetCounters::Reply counters_reply;
    string err = network::CascadeServerCall(
        ".get_counters", this, counters_req, &counters_reply);
    if (!err.empty()) return err;
    // extract the counter with validation
    if (counters_reply.size() == 0 ||
        counters_reply[0].metric_results.size() == 0) {
      return "no metric results found. likely invalid request";
    }
    // report the counter and exit
    *reply = counters_reply[0].metric_results[0];
    return "";
  }

  static Request ExampleRequest() {
    Request req;
    req.counter_name = "NewVisitCounter";
    req.interval = 60000000;
    req.metrics = { "count" };
    return req;
  }
};


ServerMethodRegister<GetCounter::Request, GetCounter::Reply, GetCounter>
    reg_GetCounter("all", ".get_counter", GetCounter::ExampleRequest());

ServerMethodRegister<GetCounters::Request, GetCounters::Reply, GetCounters>
    reg_GetCounters("all", ".get_counters", GetCounters::ExampleRequest());

}  // namespace
}  // namespace network
