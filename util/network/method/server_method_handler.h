// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)
// File created as part of refactoring rpcserver.h

// Handler for server method.

#ifndef _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_HANDLER_H_
#define _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_HANDLER_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>

#include "base/common.h"
#include "util/network/netserver.h"
#include "util/network/rpcclient.h"
#include "util/factory/factory.h"
#include "util/hash/hash_util.h"
#include "util/serial/serializer.h"
#include "util/templates/sfinae.h"

extern int gFlag_max_binary_response_size;
extern int gFlag_max_json_response_size;

namespace network {

struct tErrorMessage_JSON {
  string error_msg;
  SERIALIZE(error_msg*1);
};

enum ServerMethodStatus {
  kServerMethodStatusValid,
  kServerMethodStatusInvalid,
  kServerMethodStatusNotImplemented,
  kServerMethodStatusMax,
};

enum class MockMethodStatus {
  SUCCESS,
  FAILURE,
  UNIMPLEMENTED
};

// Returns the server message string corresponding to the status.
inline string GetServerMethodStatusMsg(ServerMethodStatus status) {
  string msg;
  switch(status) {
    case kServerMethodStatusValid : msg = "PASSED"; break;
    case kServerMethodStatusNotImplemented : msg = "NOT IMPLEMENTED"; break;
    case kServerMethodStatusInvalid : msg = "FAILED"; break;
    default: msg = "UNKNOWN"; break;
  }
  return msg;
}

// base class for all RPC Handler classes
class ServerMethodHandlerBase {
 public:
  ServerMethodHandlerBase() {};
  virtual ~ServerMethodHandlerBase() {};

  // Function to process an RPC request.
  virtual void ProcessRPC(const tConnectionInfo* connection,
                          const tServerRequestMessage& request,
                          tServerReplyMessage* response) const = 0;

  // Function to process a JSON request.
  virtual void ProcessJSON(const tConnectionInfo* connection,
                           const tServerRequestMessage& request,
                           bool debug_json,
                           tServerReplyMessage* response) const = 0;

  virtual string ProcessCascade(const void *caller, const void *input,
                                void *output) const = 0;

  // Translate an RPC query into JSON string.
  virtual string RPCToJSON(const string& message, int indent = 0) const = 0;

  // Function to retrieve the sample input value, in JSON format.
  virtual string GetSampleInput_JSON() const = 0;

  // Function to validate the server method.
  virtual ServerMethodStatus Validate(const tConnectionInfo* connection,
      const tServerRequestMessage& request, ostream& out) const = 0;

  // Function to check if the live traffic to the server method can be recorded.
  virtual bool CanRecord() const = 0;

  // Function to check if the method expects empty request. i.e. no 'q' CGI
  // param.
  virtual bool AllowEmptyRequest() const = 0;
};

// Actual RPC handler with input/output type template.
// 3 parameters:
// -- input type
// -- output type
// -- Server Method function (with operator() that takes "const tInput&" and
//    "tOutput*" and returns an error message string ("" for success).
template<class tInput, class tOutput, class tFunc>
class ServerMethodHandler : public ServerMethodHandlerBase {
 public:
  explicit ServerMethodHandler(const string& name, const tInput& sample_input)
    : name_(name), sample_input_(sample_input) {}

  virtual void ProcessRPC(const tConnectionInfo* connection,
                          const tServerRequestMessage& request,
                          tServerReplyMessage* response) const override {
    response->Clear();
    VLOG(3) << "processrpc:";
    // strutil::PrintRaw(request.message);
    // parse input from serialized string
    tInput input;
    if (!serial::Serializer::FromBinary(request.message, &input)) {
      // error during input parsing
      response->message = "RPC parsing error -- server mismatch?";
      response->success = false;
      return;
    }

    // Run the server method.
    shared_ptr<tOutput> output(new tOutput);
    tFunc f = GetServerMethod(connection, request);
    string error_msg = RunServerMethod(f, request.arg_map, input, output, response);
    if (error_msg.empty()) {
      // return output in serialized string format
      response->message = serial::Serializer::ToBinary(output);
      ASSERT_LE(response->message.size(), gFlag_max_binary_response_size)
          << "Serialized Binary response too large (size="
          << response->message.size()
          << ").  Please increase command-line flag max_binary_response_size. Request: "
          << serial::Serializer::ToJSON(input);

      response->success = true;
    } else {
      response->message = error_msg;
      response->success = false;
    }
  }

  virtual void ProcessJSON(const tConnectionInfo* connection,
                           const tServerRequestMessage& request,
                           bool debug_json,
                           tServerReplyMessage* response) const override {
    response->Clear();
    VLOG(4) << "processjson: " << request.message;
    // parse input from serialized string
    string error_msg;
    tInput input;
    if (!serial::Serializer::FromJSON(request.message, &input, {&error_msg})) {
      // error: unable to parse JSON input
      tErrorMessage_JSON err;
      err.error_msg = error_msg;
      response->message = err.ToJSON({2});
      response->success = false;
      return;
    }

    // run the server method
    shared_ptr<tOutput> output(new tOutput);
    tFunc f = GetServerMethod(connection, request);
    string error_msg_func = RunServerMethod(f, request.arg_map, input, output,
                                            response);
    if (error_msg_func.empty()) {
      // return output in JSON format
      // (if debug_json is true, format JSON output to be human-readable)
      response->message =
          serial::Serializer::ToJSON(output, {(debug_json ? 2 : 0)});
      ASSERT_LE(response->message.size(), gFlag_max_json_response_size)
          << "Serialized JSON response too large (size="
          << response->message.size()
          << ").  Please increase command-line flag max_json_response_size. Request: "
          << serial::Serializer::ToJSON(input);

      response->success = true;
    } else {
      // error: return error message to caller
      tErrorMessage_JSON err;
      err.error_msg = error_msg_func;
      response->message = err.ToJSON({2});
      response->success = false;
    }
  }

  virtual string ProcessCascade(const void *caller, const void *input,
                                void *output) const override {
    const tInput *input_data = static_cast<const tInput*>(input);
    tOutput *output_data = static_cast<tOutput*>(output);
    tFunc f;
    f.Cascade(caller);
    return RunServerMethod(f, f.arg_map(), *input_data, output_data);
  }

  // translate an RPC query into JSON string
  virtual string RPCToJSON(const string& message, int indent = 0) const override {
    tInput input;
    if (serial::Serializer::FromBinary(message, &input))
      return serial::Serializer::ToJSON(input, {indent});
    else
      return "[Error in RPC string]";
  }

  virtual string GetSampleInput_JSON() const override {
    return serial::Serializer::ToJSON(sample_input_, {2});
  }

  virtual ServerMethodStatus Validate(const tConnectionInfo* connection,
                                      const tServerRequestMessage& request,
                                      ostream& out) const override {
    tFunc f = GetServerMethod(connection, request);
    return f.RunValidate(out);
  }

  virtual bool CanRecord() const override {
      return tFunc().CanRecord();
  }

  virtual bool AllowEmptyRequest() const override {
    return tFunc().AllowEmptyRequest();
  }

 protected:
  tFunc GetServerMethod(const tConnectionInfo* connection,
                        const tServerRequestMessage& request) const {
    tFunc f;
    f.SetConnection(connection);
    f.ParseInputCookie(request.cookie);
    f.SetReferrer(request.referrer);
    f.set_http_header(request.http_header);
    f.set_arg_map(request.arg_map);

    // Set whether the call is internal or from a real user.
    string user_agent = f.GetHttpHeader("User-Agent");
    bool internal = (user_agent.empty() || user_agent == gFlag_r77_user_agent) ?
        true : false;
    f.set_internal_call(internal);
    return f;
  }

  // the shared_ptr version that either executes the shared_ptr OR the
  // pointer version of the server methods
  // runs the appropriate mock or server method
  // @return the error_msg
  string RunServerMethod(tFunc server_method,
                         const unordered_map<string, string>& arg_map,
                         const tInput& input,
                         shared_ptr<tOutput>& output,
                         tServerReplyMessage* response = nullptr) const {
    string err_msg;
    // if mock didn't run, run the real method
    if (!TryRunMock(server_method, arg_map, input, output.get(),
                    &err_msg, response)) {
      err_msg = RunHandler<tFunc>(server_method, input, output);
    }
    if (response != nullptr) response->cookies = server_method.output_cookies();
    return err_msg;
  }

  string RunServerMethod(tFunc server_method,
                         const unordered_map<string, string>& arg_map,
                         const tInput& input,
                         tOutput *output,
                         tServerReplyMessage* response = nullptr) const {
    string err_msg;
    // if mock didn't run, run the real method
    if (!TryRunMock(server_method, arg_map, input, output,
                    &err_msg, response)) {
      err_msg = server_method(input, output);
    }
    if (response != nullptr) response->cookies = server_method.output_cookies();
    return err_msg;
  }

  // @return true if the mock was run, false otherwise
  bool TryRunMock(tFunc server_method,
                  const unordered_map<string, string>& arg_map,
                  const tInput& input,
                  tOutput *output,
                  string *err_msg,
                  tServerReplyMessage* response = nullptr) const {
    MockMethodStatus status = CheckAndExecuteMock<tFunc>(arg_map, &server_method,
                                                         input, output, err_msg);
    bool success = true;
    if (status == MockMethodStatus::FAILURE) {
      stringstream ss;
      ss << "failed to generate mock for method: " << name_;
      *err_msg = ss.str();
    } else if (status == MockMethodStatus::UNIMPLEMENTED) {
      success = false;
    }
    return success;
  }

  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(Mock);
  // execute the mock method if it exists and set the output to err_msg
  // @return true if the mock succeeded
  template<typename T>
  typename std::enable_if<
    has_member_func_sig_Mock<T, MockMethodStatus(const string&, const tInput&, tOutput *) >::value,
    MockMethodStatus>::type
  CheckAndExecuteMock(const unordered_map<string, string>& arg_map, tFunc *f,
                      const tInput& input, tOutput *output, string *err_msg) const {
    const string mocktype = ParseMock(arg_map);
    if (mocktype.empty()) return MockMethodStatus::UNIMPLEMENTED;
    return f->Mock(mocktype, input, output);
  }

  template<typename T>
  typename std::enable_if<
    !has_member_func_sig_Mock<T, MockMethodStatus(const string&, const tInput&, tOutput *) >::value,
    MockMethodStatus>::type
  CheckAndExecuteMock(const unordered_map<string, string>& arg_map, tFunc *f,
                      const tInput& input, tOutput *output, string *err_msg) const {
    return MockMethodStatus::UNIMPLEMENTED;
  }

  CREATE_MEMBER_FUNC_SOFT_SIG_CHECK(Run);
  template<typename T>
  typename std::enable_if<
    has_member_func_sig_Run<T,
                            string(const tInput&, shared_ptr<tOutput>&) >::value,
                            string>::type
  RunHandler(tFunc& f, const tInput& input, shared_ptr<tOutput>& output) const {
    return f.Run(input, output);
  }

  template<typename T>
  typename std::enable_if<
    !has_member_func_sig_Run<T,
                             string(const tInput&, shared_ptr<tOutput>&) >::value,
                             string>::type
  RunHandler(tFunc& f, const tInput& input, shared_ptr<tOutput>& output) const {
    return f(input, output.get());
  }

  // @return the mock string for this server method. empty string if this server
  //   method is not valid
  string ParseMock(const unordered_map<string, string>& arg_map) const {
    string mocktype;
    const auto& it = arg_map.find("mock");
    if (it != arg_map.end()) {
      // parse the general format MetaSearch:mockname1+*:mockname2,...
      vector<string> mock_entries;
      strutil::Split(it->second, mock_entries, "+");
      for (auto& mock_entry : mock_entries) {
        // check for wildcard or this server name
        vector<string> mock_parts;
        strutil::Split(mock_entry, mock_parts, ":");
        if (mock_parts.size() == 2) {
          string method_name = strutil::Trim(mock_parts[0]);
          // check if entry applies to wildcard OR this server specifically
          if (method_name == "*" ||  method_name == name_) {
            mocktype = strutil::Trim(mock_parts[1]);
            break;
          }
        }
      }
    }
    return mocktype;
  }

 private:
  const string name_;
  tInput sample_input_;
};


// server message
class tMessageBase {
 public:
  virtual ~tMessageBase() {}
  virtual bool FromBinary(const string &s) = 0;
  virtual bool FromJSON(const string &s, string *err) = 0;
  virtual string ToBinary() =0;
  virtual string ToJSON(int indent = 0) = 0;
};

template <class T>
class tMessage : public T, public tMessageBase {
public:
  virtual ~tMessage<T>(){}
  virtual bool FromBinary(const string &s) override {
    return serial::Serializer::FromBinary(s, this);
  }
  virtual bool FromJSON(const string &s, string *err) override {
    return serial::Serializer::FromJSON(s, this, {err});
  }
  virtual string ToJSON(int indent = 0) override {
    return serial::Serializer::ToJSON(static_cast<T*>(this), {indent});
  }
  virtual string ToBinary() override {
    return serial::Serializer::ToBinary(static_cast<T*>(this));
  }
};


// new server method
class tServerMethodBase {
 public:
  virtual ~tServerMethodBase(){}
  virtual tMessageBase*  CreateRequest() const =0;
  virtual tMessageBase* CreateResponse() const =0;
  virtual string FillRequest(const tConnectionInfo& connection, const tServerRequestMessage& reqMsg, tMessageBase* request) const =0;
  virtual string operator()(const tMessageBase& request, tMessageBase* response, vector<string>* outCookies) const =0;
  virtual string ExampleJSON() const =0;
  virtual ServerMethodStatus Validate(const tMessageBase& request, ostream& out) const =0;
  virtual bool CanRecord(const tMessageBase& request) const =0;
};

template <class Req, class Res>
class tServerMethod : public tServerMethodBase{
 public:
  typedef tMessage<Req> Request;
  typedef tMessage<Res> Response;

  virtual ~tServerMethod<Req, Res>(){}

  // request & response factory
  virtual tMessageBase* CreateRequest() const override {return new Request;}
  virtual tMessageBase* CreateResponse() const override { return new Response;}

  // store connection specific data in the request. e.g. country user is coming from
  virtual string FillRequest(const tConnectionInfo& connection, const tServerRequestMessage& reqMsg, Req* request) const {
    return "";
  }
  virtual string FillRequest(const tConnectionInfo& connection, const tServerRequestMessage& reqMsg, tMessageBase* request) const override {
    Req* req = dynamic_cast<Request*>(request);
    return FillRequest(connection, reqMsg, req);
  }

  // actual server call
  virtual string operator()(const Req& request, Res* res, vector<string>* outCookie) const =0;
  virtual string operator()(const tMessageBase& request, tMessageBase* response, vector<string>* outCookies) const override {
    const Req& req = dynamic_cast<const Request&>(request);
    Response* res = dynamic_cast<Response*>(response);
    return operator()(req, res, outCookies);
  }

  // sample request
  virtual void Example(Req* req) const {}
  virtual string ExampleJSON() const override {
    Request req;
    Example(&req);
    return req.ToJSON(2);
  }

  // Function to run the validator for the server method.
  virtual ServerMethodStatus Validate(const Req &req, ostream& out) const{
    out << "Not implemented yet!" << endl;
    return kServerMethodStatusNotImplemented;
  }
  virtual ServerMethodStatus Validate(const tMessageBase& request, ostream& out) const override {
    const Req& req = dynamic_cast<const Request&>(request);
    return Validate(req, out);
  }

  // Returns true if a server method does not contain sensitive data and
  // live traffic to it can be recorded.
  virtual bool CanRecord(const Req &req) const {
    return true;
  }
  virtual bool CanRecord(const tMessageBase& request) const override {
    const Req& req = dynamic_cast<const Request&>(request);
    return CanRecord(req);
  }
};

// new server method handler
class ServerMethodHandler2 : public ServerMethodHandlerBase {
 public:
  ServerMethodHandler2(shared_ptr<tServerMethodBase> m)
    : ServerMethod(m) {}
  virtual void ProcessRPC(const tConnectionInfo* connection,
                          const tServerRequestMessage& reqMsg,
                          tServerReplyMessage* response) const override;

  virtual void ProcessJSON(const tConnectionInfo* connection,
                           const tServerRequestMessage& request,
                           bool debug_json,
                           tServerReplyMessage* response) const override;

  virtual string RPCToJSON(const string& message, int indent = 0) const override;
  virtual string GetSampleInput_JSON() const override;
  virtual ServerMethodStatus Validate(const tConnectionInfo* connection,
      const tServerRequestMessage& request, ostream& out) const override;
  virtual bool CanRecord() const override {return true; };

  virtual bool AllowEmptyRequest() const override { return false; }

  virtual string ProcessCascade(const void *caller, const void *input, void *output) const override {
    return "";
  }

 private:
  shared_ptr<tServerMethodBase> ServerMethod;
};

// Collection of all the server methods for a particular server.
// We assume all server methods will be registered before the server is started
// and no new ones would be added after that. This allows us to do all gets
// on method handlers without locking the mutex. In case this changes in the
// future, we will have to modify the code accordingly.
class ServerMethodHandlerCollection :
    public Factory<ServerMethodHandlerCollection> {
 public:
  typedef unordered_map<string, shared_ptr<ServerMethodHandlerBase>,
      ::hash::string_casefold_hash, ::hash::string_casefold_eq> HandlerMap;

  ServerMethodHandlerCollection() {}

  ServerMethodHandlerCollection(const ServerMethodHandlerCollection& collection) {
    AddCollection(collection);
  }

  // Returns the proxy for the method collection for the given server.
  static shared_proxy GetCollectionForServer(const string& server_name) {
    shared_proxy proxy = make_shared(server_name);
    if (proxy == nullptr) {
      bind(server_name, [] { return new ServerMethodHandlerCollection(); });
      proxy = make_shared(server_name);
    }
    ASSERT_NOTNULL(proxy) << "Could not get collection for server " << server_name;
    return proxy;
  }

  // @return look across all methods and return the server handler if the method
  //   name is found. if not found, return nullptr
  static ServerMethodHandlerBase* GetMethod(const string& method_name) {
    struct Initialize {
      shared_ptr<ServerMethodHandlerCollection> operator()() {
        vector<string> keys;
        Factory<ServerMethodHandlerCollection>::append_keys(keys);
        shared_ptr<ServerMethodHandlerCollection>
          method_collection(new ServerMethodHandlerCollection);
        for (auto& key : keys) {
          method_collection->AddCollection(*GetCollectionForServer(key));
        }
        return method_collection;
      }
    };
    static shared_ptr<ServerMethodHandlerCollection> all_methods = Initialize()();
    ServerMethodHandlerBase *server_method = all_methods->GetHandler(method_name);
    // can be nullptr
    return server_method;
  }

  // template with 3 parameters:
  // - input type
  // - output type
  // - RPC function (with operator() that takes "const tInput&" and "tOutput *"
  //                 and returns 0 (for success) or non-zero error code)
  template<class tInput, class tOutput, class tFunc>
  void Register(const string& name, const tInput& sample_input) const {
    // sanity checks
    ASSERT(name.size() > 0) << "opname cannot be empty.";
    ASSERT(name.find(' ') == string::npos) << "opname cannot contain space characters.";
    ASSERT(name != "GET")  << "opname cannot be 'GET'.";
    ASSERT(name != "POST") << "opname cannot be 'POST'.";
    ASSERT(name != "URL")  << "opname cannot be 'URL'.";
    ASSERT(name != "URL")  << "opname cannot be 'URL'.";
    ASSERT(name[0] != '_') << "opname cannot start with '_'.";

    lock_guard<std::mutex> l(mutex_);
    // Check for duplicates.
    ASSERT(handlers_.find(name) == handlers_.end()) << "Duplicate handler op-name: " << name;

    LOG(INFO) << "Registering method: " << name;
    handlers_[name] = shared_ptr<ServerMethodHandlerBase>(
      new ServerMethodHandler<tInput, tOutput, tFunc>(name, sample_input));
  }

  void Register(const string& name, shared_ptr<tServerMethodBase> method) const {
    // sanity checks
    ASSERT(name.size() > 0) << "opname cannot be empty.";
    ASSERT(name.find(' ') == string::npos) << "opname cannot contain space characters.";
    ASSERT(name != "GET")  << "opname cannot be 'GET'.";
    ASSERT(name != "POST") << "opname cannot be 'POST'.";
    ASSERT(name != "URL")  << "opname cannot be 'URL'.";
    ASSERT(name != "URL")  << "opname cannot be 'URL'.";
    ASSERT(name[0] != '_') << "opname cannot start with '_'.";

    lock_guard<std::mutex> l(mutex_);
    // Check for duplicates.
    ASSERT(handlers_.find(name) == handlers_.end()) << "Duplicate handler op-name: " << name;

    LOG(INFO) << "Registering method: " << name;
    handlers_[name] = shared_ptr<ServerMethodHandlerBase>(
        new ServerMethodHandler2(method));
  }

  // Returns the handler for the input method name.
  ServerMethodHandlerBase* GetHandler(const string& name) const {
    const auto iter = handlers_.find(name);
    if (iter != handlers_.end()) return iter->second.get();
    return nullptr;
  }

  // Returns the registered name and the handler for the input method name.
  pair<string, ServerMethodHandlerBase*> GetHandlerAndName(
      const string& name) const {
    const auto iter = handlers_.find(name);
    if (iter != handlers_.end()) return {iter->first, iter->second.get()};

    return {name, nullptr};
  }

  const HandlerMap& handlers() const { return handlers_; }

  int size() const { return handlers_.size(); }
  bool empty() const { return handlers_.empty(); }

  // Returns the method names.
  template<typename C>
  C MethodNames() const {
    C names;
    for (const auto& p : handlers_) names.insert(names.end(), p.first);
    return names;
  }

  // Adds the given collection to the current collection.
  void AddCollection(const ServerMethodHandlerCollection& collection) {
    for (const auto& p : collection.handlers_) handlers_.insert(p);
  }

 private:
  mutable HandlerMap handlers_;
  mutable std::mutex mutex_;
};

// Structure to register a new server method with the given servers.
// A single method can be registered with multiple servers by providing
// Comma separated list of server names.
template<class tInput, class tOutput, class tFunc>
struct ServerMethodRegister {
  ServerMethodRegister(const string& server_names, const string& method_name,
                       const tInput& sample_input) {
    vector<string> servers;
    ASSERT(strutil::SplitString(server_names, ",", &servers));
    for (const string& server_name : servers) {
      VLOG(3) << __PRETTY_FUNCTION__ << " S: " << server_name << ", M: " << method_name;
      ServerMethodHandlerCollection::shared_proxy proxy =
          ServerMethodHandlerCollection::GetCollectionForServer(server_name);
      proxy->Register<tInput, tOutput, tFunc>(method_name, sample_input);
      // Pin this proxy to ensure it is never destroyed.
      ServerMethodHandlerCollection::pin(proxy);
    }
  }
};

template <class T>
struct tServerMethodRegister {
  tServerMethodRegister(const string& server_names, const string& method_name) {
    vector<string> servers;
    ASSERT(strutil::SplitString(server_names, ",", &servers));
    shared_ptr<tServerMethodBase> method(new T);
    for (const string& server_name : servers) {
      VLOG(3) << __PRETTY_FUNCTION__ << " S: " << server_name << ", M: " << method_name;
      ServerMethodHandlerCollection::shared_proxy proxy =
          ServerMethodHandlerCollection::GetCollectionForServer(server_name);
      proxy->Register(method_name, method);
      // Pin this proxy to ensure it is never destroyed.
      ServerMethodHandlerCollection::pin(proxy);
    }
  }
};

}  // namespace network


#endif  // _PUBLIC_UTIL_NETWORK_METHOD_SERVER_METHOD_HANDLER_H_
