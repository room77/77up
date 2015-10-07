//
// Copyright 2007 OpTrip, Inc.
//

#ifndef _PUBLIC_UTIL_NETWORK_RPCSERVER_H_
#define _PUBLIC_UTIL_NETWORK_RPCSERVER_H_

#include <mutex>
#include <memory>

#include "util/network/netserver.h"
#include "util/network/rpc_datatypes.h"
#include "util/network/method/server_method_handler.h"
#include "util/network/util.h"
#include "util/serial/serializer.h"
#include "util/file/shared_writer.h"
#include "util/time/utime.h"

// RPCServer/RPCClient supports remote procedure call functionality
//
// An RPC connection may contain multiple transactions.  Each transaction
//   starts with a variable-length op-name (not to exceed 100 characters),
//   followed by space character, followed by encoded message (usually defined
//   by Serializer).

extern string gFlag_webroot;
extern bool gFlag_debug_json;

namespace network {

class ServerMethodForwardCollection;

struct tPendingMethod {
  int id;
  time_t timestamp;
  string opname;
  bool is_json;
  string input_string;
};

// Stats for each server method.
struct tMethodStat {
  int count;
  int time_min, time_max;  // min/max running time
  long long time_total;  // in order to calculate average running time
  tMethodStat() : count(0), time_min(0), time_max(0), time_total(0) {}
};

// Class to implement RPC server.
class RPCServer : public NetServer {
 public:
  // Comma separated list of names for which the server methods are allowed for
  // this server. The first name is assumed to be the name of the server.
  RPCServer(const string& names = "server") : NetServer(), max_id_(0) {
    Initialize(names);
  }

  virtual ~RPCServer();

  util::SharedWriter& InputWriter();

  virtual void StartServer();

  // template with 3 parameters:
  // - input type
  // - output type
  // - RPC function (with operator() that takes "const tInput&" and "tOutput *"
  //                 and returns 0 (for success) or non-zero error code)
  template<class tInput, class tOutput, class tFunc>
  void RegisterHandler(const char *opname, const tInput& sample_input) {
    method_collection_->Register<tInput, tOutput, tFunc>(opname,
                                                         sample_input);
  }

  void RegisterHandler(const char *opname, shared_ptr<tServerMethodBase> method) {
    method_collection_->Register(opname, method);
  }

  // print sample proxy configuration lines for Apache 2.0 (launch preparation)
  void PrintProxyConfig() const;

  // Validates the server testing all its interfaces.
  virtual ServerMethodStatus Validate(
      const tConnectionInfo *connection, const tServerRequestMessage& request,
      ostream& out);

  // Validates the server testing all its interfaces.
  virtual ServerMethodStatus ValidateMethod(const string& method,
                                            const tConnectionInfo *connection,
                                            const tServerRequestMessage& request,
                                            ostream& out);

 private:
  // Initialize the RPC server.
  void Initialize(const string& names);

  // check if all bytes have been received
  virtual bool RequestIsComplete(const string& r,
                                 unsigned int *request_size) const;

  // allow multiple requests per connection
  virtual bool OneRequestPerConnection() const { return false; }

  // process a request
  virtual string ProcessRequest(const string& request,
                                bool *keep_alive,
                                const tConnectionInfo *connection);

  string ProcessRPCRequest(const string& request,
                           const tConnectionInfo *connection,
                           const tServerRequestMessage& request_params = {});

  // construct the root web form for http debug interface
  string ConstructRootForm(const unordered_map<string, string>& arg_map) const;

  // server usage tracking
  int TrackUsage_Begin(const char *opname, bool is_json, const string& input);
  void TrackUsage_End(int id);
  string RedirectPage(const string& url) const;
  string ReportUsage();
  virtual bool CheckHealth() const;

  Time init_time_;
  int max_id_;
  string name_;
  shared_ptr<ServerMethodHandlerCollection> method_collection_;
  shared_ptr<ServerMethodForwardCollection> forward_method_collection_;
  unordered_map<int, tPendingMethod *> in_progress_;
  unordered_map<string, tMethodStat> access_count_;
  mutex mutex_stat_;
};

}  // namespace network

#endif  // _PUBLIC_UTIL_NETWORK_RPCSERVER_H_
