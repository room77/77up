#include <algorithm>
#include <functional>
#include <random>
#include <signal.h>
#include <sstream>
#include <sys/time.h>

#include "util/network/rpcserver.h"

#include "base/args/args.h"
#include "base/signal_handler.h"
#include "util/file/file.h"
#include "util/network/method/common_methods/params/param_editor.h"
#include "util/network/method/server_method_forward.h"
#include "util/network/webserver.h"
#include "util/serial/encoding/encoding.h"
#include "util/serial/serializer.h"
#include "util/string/strutil.h"
#include "util/time/simple_timer.h"
#include "util/thread/thread_stack.h"

FLAG_string(webroot, ".", "public web directory");

// debug_json can be overridden by &d=1 flag
FLAG_bool(debug_json, false, "format JSON output to be human-readable");

// /data/output is symlinked to the large local filesystem on prod
FLAG_string(playback_input_fn, "/data/output/input_playback",
            "record the input playback to this file");

FLAG_string(playback_output_fn, "/data/output/output_playback",
            "record the output playback to this file");

// support an hour of recording
FLAG_int(max_recording_sec, 3600, "the maximum seconds a recording is allowed to be");

// Validate the server only.
FLAG_bool(validate_server, false,
          "Validates the server and exits if set.");


namespace network {

// Support functions for input recording.
namespace {

bool& IsRecording() {
  static bool is_recording = false;
  return is_recording;
}
int& RecordingStart() {
  static int recording_start_ts = 0;
  return recording_start_ts;
}

// return a random integer between 0 and (max - 1)
inline int RandomInteger(int max) {
  int r = static_cast<int>(1.0 * rand() / (RAND_MAX + 1.0) * max);
  if (r < 0 || r >= max)  // this shouldn't happen
    r = 0;
  return r;
}

bool CheckOutgoingNetworkTraffic() {
  const static vector<string> servers = { "www.google.com", "www.facebook.com",
      "www.yahoo.com", "www.wikipedia.com", "www.nbc.com", "www.youtube.com",
      "www.twitter.com", "www.linkedin.com", "www.bing.com" };

  struct InitRand {
    std::function<int ()> operator ()(int size) {
      std::random_device rd;
      std::mt19937 engine(rd());
      std::uniform_int_distribution<> dis(0, size);
      return std::bind(dis, engine);
    }
  };

  static std::function<uint16_t ()> rand = InitRand()(servers.size() - 1);

  bool status = NetworkUtil::Ping(servers[rand()]);
  // If pinging one server fails, try pinging another to be sure.
  if (!status) status = NetworkUtil::Ping(servers[rand()]);
  return status;
}

}  // namespace

RPCServer::~RPCServer() {}

void RPCServer::Initialize(const string& names) {
  vector<string> n;
  ASSERT(strutil::SplitString(names, ",", &n));
  name_ = n.front();

  // Always add methods registered with "all".
  n.push_back("all");
  method_collection_.reset(new ServerMethodHandlerCollection);
  forward_method_collection_.reset(new ServerMethodForwardCollection);
  for (const string& name : n) {
    ServerMethodHandlerCollection::shared_proxy methods =
        ServerMethodHandlerCollection::GetCollectionForServer(name);
    ASSERT_NOTNULL(methods);
    method_collection_->AddCollection(*methods);

    ServerMethodForwardCollection::shared_proxy fwd_methods =
         ServerMethodForwardCollection::GetCollectionForServer(name);
    ASSERT_NOTNULL(fwd_methods);
    forward_method_collection_->AddCollection(*fwd_methods);
  }
  // register the signal handle so will call shutdown
  if (!::base::SignalHandler::Instance().IsHandlerRegistered(SIGTERM)) {
    ASSERT(::base::SignalHandler::Instance().Register(
        SIGTERM,
        [&] () {
          LOG(INFO) << "Preparing to shutdown...";
          prepare_shutdown_ = true;
          shutdown_loop_ = false;
          CloseMainSocket();
          // ignore the signal and wait for shutdown
          signal(SIGTERM, SIG_IGN);
        },
        SA_RESTART)) << "Failed to register SIGTERM in rpcserver";
  }
}

util::SharedWriter& RPCServer::InputWriter() {
  stringstream ss;
  ss << gFlag_playback_input_fn << "_" << portnum_;
  static util::SharedWriter input_writer(ss.str());
  return input_writer;
}

void RPCServer::StartServer() {
  if (gFlag_validate_server) {
    tConnectionInfo connection;
    ServerMethodStatus status =
      Validate(&connection, tServerRequestMessage(), cout);
    ASSERT_EQ(status, kServerMethodStatusValid);
    return;
  }
  NetServer::StartServer();
}

// check if all bytes have been received
bool RPCServer::RequestIsComplete(const string& r,
                                  unsigned int *request_size) const {
  const char *s = r.c_str();

  if (!strncmp(s, "POST", 4) || !strncmp(s, "GET", 3)) {
    // input is in JSON format via http
    return WebServer::HttpRequestIsComplete(r, request_size, NULL, NULL, NULL,
                                            NULL, NULL, NULL, NULL, NULL);
  } else {
    // Input is in our internal RPC serialized format.
    if(RPCMessageIsComplete(r)) {
      *request_size = r.size();
      return true;
    }
    return false;
  }
}

// process a request
string RPCServer::ProcessRequest(const string& request,
                                 bool *keep_alive,
                                 const tConnectionInfo *connection) {
  unsigned int request_len;
  if (RequestIsComplete(request, &request_len)) {
    const char *s = request.c_str();

    if (!strncmp(s, "POST", 4) || !strncmp(s, "GET", 3)) {
      VLOG(4) << "processwebrequest:" << request;
      // input is in JSON format via http
      ::util::time::SimpleTimer timer;
      timer.Start();

      tServerReplyMessage reply;
      reply.success = true;
      bool reply_http_response = true;

      string return_content_type = "text/plain";

      unsigned int request_size;
      string url, cgi_arguments, input_cookie, referrer;
      bool accept_gzip, is_post;

      string output_cookie;
      string callback_hack;

      unordered_map<string, string> http_header;

      vector<string> empty_cookies;

      if (WebServer::HttpRequestIsComplete(request, &request_size,
                                           &url, &cgi_arguments,
                                           &input_cookie, &referrer,
                                           &accept_gzip, keep_alive,
                                           &is_post, &http_header)) {
        // log some info. Usage is called by HAProxy constantly to see server
        // status. This pollutes our logs too much. Suppressing logging for it
        // for now.
        if (url != "/_usage")
          LogCaller(connection->caller_id,
                    WebServer::HttpRequestSummary(url, cgi_arguments, is_post));

        // url (minus the leading /) is the opname
        const char *opname = url.c_str();
        if (*opname == '/') opname++;

        if (*opname == '\0') {
          // request the root page
          return_content_type = "text/html";
          unordered_map<string, string> arg_map;
          if (!NetworkUtil::ParseCGIArguments(cgi_arguments, &arg_map))
            arg_map.clear();
          reply.message = ConstructRootForm(arg_map);
        } else if (!strcmp(opname, "URL")) {
          // request URL redirect
          string target_url = strutil::GetTrimmedString(cgi_arguments);
          if (target_url.empty())
            return WebServer::ConstructHttpResponse
                     (WebServer::gHttpResponse_NotFound, "", "",
                      accept_gzip, empty_cookies, 0);
          return_content_type = "text/html";
          reply.message = RedirectPage(target_url);
        } else if (!strcmp(opname, "_usage")) {
          // request usage page
          return_content_type = "text/html";
          reply.message = ReportUsage();
        } else if (!strcmp(opname, "_status")) {
          // request status page
          return_content_type = "text/plain";
          ifstream in("/proc/self/status");
          reply.message = string(istreambuf_iterator<char>(in),
                                 istreambuf_iterator<char>());
        } else if (!strcmp(opname, "_startrecord")) {
          return_content_type = "text/plain";
          IsRecording() = true;
          // starts writing again at the beginning of the file
          InputWriter().Reset();
          // refreshes the recording timestamp
          RecordingStart() = time(NULL);
          reply.message = "Recording started";
        } else if (!strcmp(opname, "_stoprecord")) {
          return_content_type = "text/plain";
          IsRecording() = false;
          reply.message = "Recording stopped";
        } else if (!strcmp(opname, "_param")) {
          // view / edit parameters
          return_content_type = "text/html";
          reply.message = params::ParamEditor();
        } else if (!strcmp(opname, "_shutdownloop")) {
          // prepare to shutdown this server as well as its loop script
          return_content_type = "text/plain";
          reply.message = "Preparing to shutdown...\n";
          prepare_shutdown_ = true;
          shutdown_loop_ = true;
          CloseMainSocket();
        } else if (!strcmp(opname, "_shutdown")) {
          // prepare to shutdown
          return_content_type = "text/plain";
          reply.message = "Preparing to shutdown...\n";
          prepare_shutdown_ = true;
          shutdown_loop_ = false;
          CloseMainSocket();
        } else if (!strcmp(opname, "_rpc")) {
          // special RPC request wrapped in HTTP format
          // (client counterpart is HttpClient::RPCWrappedInHttpPost())
          // We assume the arg_map is set in the request directly.
          tServerRequestMessage request_params("", "", input_cookie,
                                               referrer, http_header);

          string rpc_reply = ProcessRPCRequest(cgi_arguments, connection,
                                               request_params);
          return WebServer::ConstructHttpResponse(WebServer::gHttpResponse_OK,
                                                  rpc_reply,
                                                  "application/octet-stream",
                                                  accept_gzip,
                                                  empty_cookies, 0);
        } else if (!strcmp(opname, "_validate")) {
          return_content_type = "text/plain";
          tServerRequestMessage query("", request, input_cookie, referrer);
          ostringstream out;
          unordered_map<string, string> arg_map;
          if (!NetworkUtil::ParseCGIArguments(cgi_arguments, &arg_map))
            arg_map.clear();

          if (arg_map.find("method") != arg_map.end())
            ValidateMethod(arg_map["method"], connection, query, out);
          else
            Validate(connection, query, out);

          reply.message = out.str();
        } else if (!strcmp(opname, "_health")) {  // Health of the server.
          return_content_type = "text/plain";
          reply.message = "OK";
          bool status = CheckHealth();
          if (!status)
            return WebServer::ConstructHttpResponse(
              WebServer::gHttpResponse_ServerError, "", "", accept_gzip, empty_cookies, 0);
        } else if (!strcmp(opname, "_threads")) {  // Threads of the server.
          return_content_type = "text/plain";
          reply.message =
              util::threading::ThreadStack::Instance().GetTraceForAllThreads();
        }
        /*
        else if (!strcmp(opname, "_reload")) {
          // re-initialize from server configuration file
          CodeTranslator::ServerConfig::Instance().Init();
          return_content_type = "text/plain";
          reply.message = "Server config file reloaded.\n";
        }
        */
        else {
          pair<string, ServerMethodHandlerBase*> p =
              method_collection_->GetHandlerAndName(opname);
          // look up the appropriate handler
          if (p.second == nullptr) {
            // undefined opname

            // assume this is a file request
            string filename = gFlag_webroot;
            if (!(filename.empty()))
              if (filename[filename.size() - 1] != '/')
                filename += '/';
            filename += opname;

            string content;
            if (file::ReadFileToString(filename, &content)) {
              const int max_cache_seconds = 3600;  // todo: allow other values
              return WebServer::ConstructHttpResponse
                       (WebServer::gHttpResponse_OK,
                        content,
                        WebServer::GuessContentType(opname),
                        accept_gzip, empty_cookies, max_cache_seconds);
            } else {
              return WebServer::ConstructHttpResponse
                (WebServer::gHttpResponse_NotFound, "", "",
                 accept_gzip, empty_cookies, 0);
            }
          } else {
            // extract JSON input message from cgi argument
            //   variable q: JSON message
            //   variable d: if d=1, result is formatted to be human-readable

            return_content_type = "text/plain";

            unordered_map<string, string> arg_map;
            if (!NetworkUtil::ParseCGIArguments(cgi_arguments, &arg_map)) {
              // error parsing CGI arguments
              tErrorMessage_JSON err;
              err.error_msg = "Unable to parse CGI arguments.";
              reply.message = err.ToJSON({2});
              reply.success = false;
            } else if (arg_map.find("q") == arg_map.end() &&
                arg_map.find("no_input") == arg_map.end()) {
              tErrorMessage_JSON err;
              err.error_msg = "Missing input (q).";
              reply.message = err.ToJSON({2});
              reply.success = false;
            } else {
              unordered_map<string, string>::const_iterator itr;

              // if "d=1" is set, enable debug mode (format JSON output to be
              // human-readable)
              bool debug_json = gFlag_debug_json;
              if ((itr = arg_map.find("d")) != arg_map.end())
                debug_json = (itr->second == "1");

              // if "callback=..." is set, treat it as callback function to wrap
              // JSON result in (part of a hack to bypass cross-site scripting
              // restrictions)
              if ((itr = arg_map.find("callback")) != arg_map.end())
                callback_hack = itr->second;

              // Check if we want to add a header to the reply.
              if ((itr = arg_map.find("header")) != arg_map.end())
                reply_http_response = (itr->second == "1");

              // if "e=..." is set, force an error response after timeout
              //   --- this is to assist with frontend error handling debugging
              // (e=0: force an immediate error response)
              // (e=100: wait for 100 seconds and then give an error response)
              if ((itr = arg_map.find("e")) != arg_map.end()) {
                int force_error = atoi(itr->second.c_str());
                if (force_error > 180)
                  force_error = 180;
                if (force_error > 0)
                  sleep(force_error);  // wait for this many seconds
                // return an error
                tErrorMessage_JSON err;
                err.error_msg = "Caller-requested error.";
                reply.message = err.ToJSON({2});
                reply.success = false;
              } else {
                // proper query processing

                // if "y=..." is set, randomly delay a number of seconds
                // before processing query (this is for debugging purposes)
                if ((itr = arg_map.find("y")) != arg_map.end()) {
                  int delay_seconds = RandomInteger(atoi(itr->second.c_str()));
                  if (delay_seconds > 180)
                    delay_seconds = 180;
                  if (delay_seconds > 0)
                    sleep(delay_seconds);  // wait for this many seconds
                }

                const string& json_input = arg_map["q"];

                // keep track of server usage
                const string& opname = p.first;
                int id = TrackUsage_Begin(opname.c_str(), true, json_input);
                tServerRequestMessage query("", json_input, input_cookie,
                                            referrer, http_header, arg_map);

                if (IsRecording() && p.second->CanRecord() &&
                    (time(NULL) - RecordingStart()) <= gFlag_max_recording_sec) {
                  struct timeval current_time;
                  gettimeofday(&current_time, NULL);
                  stringstream ss;

                  // \n \r may be sent via manual user input (typically for
                  // debugging). Convert them to spaces to preserve one entry
                  // per line property.
                  string msg = query.message;
                  for (int i = 0; i < msg.size(); ++i)
                    if (msg[i] == '\n' || msg[i] == '\r') msg[i] = ' ';
                  ss << fixed << setprecision(0)
                     << current_time.tv_sec*1000.0 + current_time.tv_usec/1000.0
                     << "\t" << opname
                     << "\t" << msg;
                  InputWriter().Write(ss.str());
                }
                p.second->ProcessJSON(connection, query, debug_json, &reply);

                // @TODO(recording/playback): record timestamp and reply.message

                TrackUsage_End(id);
              }
            }
          }
        }
      }

      if (!callback_hack.empty()) {
        reply.message = callback_hack + "(" + reply.message + ");";
        return_content_type = "application/javascript";
      }

      timer.Stop();
      VLOG(4) << "Request: " << request << ", time: " << timer.GetDurationSec()
             << "s";

      if (reply_http_response) {
        // return message as http response
        const int max_cache_seconds = 0;  // todo: allow other values
        return WebServer::ConstructHttpResponse(WebServer::gHttpResponse_OK,
                                                reply.message,
                                                return_content_type,
                                                accept_gzip,
                                                reply.cookies,
                                                max_cache_seconds);
      } else {
        return reply.message;
      }
    } else {
      // input is in our internal RPC serialized format

      if (keep_alive != NULL)
        *keep_alive = true;  // allow multiple calls per connection

      return ProcessRPCRequest(request, connection);
    }
  }

  VLOG(3) << "Incomlete Request:" << serial::encoding::EscapeString(request);
  return "";
}

string RPCServer::ProcessRPCRequest(const string& request,
                                    const tConnectionInfo *connection,
                                    const tServerRequestMessage& request_params) {
  VLOG(4) << "processrpcrequest:" << serial::encoding::EscapeString(request);
  // strutil::PrintRaw(request);

  // attempt to parse request as "opname followed by message"
  tServerRequestMessage rpc_incoming;
  tServerReplyMessage rpc_reply;

  // parse message as a simple RPC-serialized string
  if (serial::Serializer::FromBinaryPrependedSize(request, &rpc_incoming)) {
    LogCaller(connection->caller_id,
              string("rpc request -- ") + rpc_incoming.opname);

    // look up the appropriate handler
    pair<string, ServerMethodHandlerBase*> p =
        method_collection_->GetHandlerAndName(rpc_incoming.opname);
    if (p.second == nullptr)
      LOG(INFO) << "Undefined op-name received: " << rpc_incoming.opname;
    else {
      ServerMethodHandlerBase *h = p.second;

      // keep track of server usage
      const string& opname = p.first;
      int id = TrackUsage_Begin(opname.c_str(), false,
                                h->RPCToJSON(rpc_incoming.message));
      if (IsRecording() && h->CanRecord() &&
          (time(NULL) - RecordingStart()) <= gFlag_max_recording_sec) {
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        stringstream ss;
        ss << fixed << setprecision(0)
           << current_time.tv_sec*1000.0 + current_time.tv_usec/1000.0
           << "\t" << opname
           << "\t" << h->RPCToJSON(rpc_incoming.message, -1);
        InputWriter().Write(ss.str());
      }
      // Merge metadata for the request if present.
      rpc_incoming.MergeMetaData(request_params);
      h->ProcessRPC(connection, rpc_incoming, &rpc_reply);
      TrackUsage_End(id);
    }
  } else LogCaller(connection->caller_id, "Invalid rpc request");

  // return string is the raw string further encoded in RPC format
  return serial::Serializer::ToBinaryPrependSize(rpc_reply);
}


ServerMethodStatus RPCServer::Validate(const tConnectionInfo *connection,
                                       const tServerRequestMessage& request,
                                       ostream& out) {
  ServerMethodStatus res = kServerMethodStatusValid;
  out << "Validating server..." << endl;
  vector <vector<string> > handlers_status(kServerMethodStatusMax);
  for (const auto& handler : method_collection_->handlers()) {
    ServerMethodStatus status =
        ValidateMethod(handler.first, connection, request, out);
    handlers_status[status].push_back(handler.first);
  }
  out << endl;

  // Log handlers that failed.
  const vector<string> & handlers_failed =
      handlers_status[kServerMethodStatusInvalid];
  if (handlers_failed.size()) {
    out << endl;
    out << "FAILURE: " << handlers_failed.size() << " of "
        << method_collection_->size() << " methods failed." << endl;
    out << "Methods Failed:";
    for_each(handlers_failed.begin(), handlers_failed.end(),
             [&out](const string& name) { out << "  " << name; });
    if (handlers_failed.size() * 2 >= method_collection_->size())
      res = kServerMethodStatusInvalid;
    out << endl;
  }

  // Log handlers that are not implemented.
  const vector<string> & handlers_not_implemented =
      handlers_status[kServerMethodStatusNotImplemented];
  if (handlers_not_implemented.size()) {
    out << endl;
    out << "Not Implemented: " << handlers_not_implemented.size() << " of "
        << method_collection_->size() << " methods not implemented." << endl;
    out << "Methods Not Implemented:";
    for_each(handlers_not_implemented.begin(), handlers_not_implemented.end(),
             [&out](const string& name) { out << "  " << name; });
    if (handlers_not_implemented.size() * 2 >= method_collection_->size())
      res = kServerMethodStatusNotImplemented;
    out << endl;
  }

  // Log handlers that passed.
  const vector<string> & handlers_passed =
      handlers_status[kServerMethodStatusValid];
  if (handlers_passed.size()) {
    out << endl;
    out << "PASSED: " << handlers_passed.size() << " of "
        << method_collection_->size() << " methods passed." << endl;
    out << "Methods Passed:";
    for_each(handlers_passed.begin(), handlers_passed.end(),
             [&out](const string& name) { out << "  " << name; });
    out << endl;
  }
  out << endl << "FINAL STATUS: " << GetServerMethodStatusMsg(res) << endl;
  return res;
}

ServerMethodStatus RPCServer::ValidateMethod(const string& method,
                                             const tConnectionInfo *connection,
                                             const tServerRequestMessage& request,
    ostream& out) {
  ServerMethodHandlerBase* handler = method_collection_->GetHandler(method);
  if (method.empty() || (handler == nullptr)) {
    out << "Invalid Method: " << method << endl;
    return kServerMethodStatusNotImplemented;
  }

  out << endl << "# " << method  << ": Validating... #" << endl;
  ServerMethodStatus status = handler->Validate(connection, request, out);
  string msg = GetServerMethodStatusMsg(status);
  out << "# " << method << ": " << msg << " #" << endl;
  return status;
}

// server usage tracking
int RPCServer::TrackUsage_Begin(const char *opname, bool is_json,
                                const string& input) {
  lock_guard<mutex> l(mutex_stat_);

  // keep track of this pending method call
  tPendingMethod *p = new tPendingMethod;
  p->id = ++max_id_;
  p->timestamp = time(NULL);
  p->opname = opname;
  p->is_json = is_json;
  p->input_string = input;
  in_progress_[p->id] = p;

  return p->id;
}

void RPCServer::TrackUsage_End(int id) {
  lock_guard<mutex> l(mutex_stat_);

  // remove internal record of this method call
  unordered_map<int, tPendingMethod *>::iterator i = in_progress_.find(id);
  ASSERT(i != in_progress_.end()) << "internal error -- invalid id provided";
  tPendingMethod *p = i->second;
  in_progress_.erase(i);

  // keep track of method running time
  int time_elapsed = time(NULL) - p->timestamp;
  tMethodStat& method_stat = access_count_[p->opname];
  if (method_stat.count == 0) {
    method_stat.count = 1;
    method_stat.time_min = time_elapsed;
    method_stat.time_max = time_elapsed;
    method_stat.time_total = time_elapsed;
  }
  else {
    ++(method_stat.count);
    if (time_elapsed < method_stat.time_min)
      method_stat.time_min = time_elapsed;
    if (time_elapsed > method_stat.time_max)
      method_stat.time_max = time_elapsed;
    method_stat.time_total += time_elapsed;
  }

  delete p;
}

string RPCServer::RedirectPage(const string& url) const {
  stringstream ss;
  ss << "<html>\n<head>\n<meta http-equiv=\"Refresh\" content=\"0;url="
     << url << "\">\n</head>\n<body>\n<a href=\"" << url
     << "\">Loading...</a><p>\n</body>\n</html>\n";
  return ss.str();
}

string RPCServer::ReportUsage() {
  lock_guard<mutex> l(mutex_stat_);

  Time curr_time;
  vector<int> id_list;
  id_list.reserve(in_progress_.size());
  for (unordered_map<int, tPendingMethod *>::const_iterator
         i = in_progress_.begin();
       i != in_progress_.end();
       ++i)
    id_list.push_back(i->first);
  sort(id_list.begin(), id_list.end());

  // dump all pending methods
  stringstream html;
  html << "<html><head><title>Usage Report - R77 RPC Server</title>"
       << "<link rel=\"icon\" type=\"image/png\" "
       << "href=\"/images/icons/fav-internal.png\" />\n"
       << "<script language=\"JavaScript\">"
       << "<!--\n"
       << "function toggleinput(seq) {\n"
       << "  var label = document.getElementById(\"toggle\" + seq);\n"
       << "  var txt = document.getElementById(\"inputstr\" + seq);\n"
       << "  if (txt.style.display == \"none\") {\n"
       << "    txt.style.display = \"block\";\n"
       << "    label.innerHTML = \"Hide\";\n"
       << "  }\n"
       << "  else {\n"
       << "    txt.style.display = \"none\";\n"
       << "    label.innerHTML = \"Show\";\n"
       << "  }\n"
       << "}\n"
       << "// -->\n"
       << "</script>\n"
       << "</head>\n<body>\n"
       << "Server started: " << Time::PrintDuration(curr_time - init_time_)
       << " ago ("
       << init_time_.ToLocalTime(Timezone::PST()).Print() << " PST)<p>\n"
       << "Total number of calls: " << max_id_ << "<p>\n"
       << ("<table border=1><tr align=center><td><i>method</i></td>"
           "<td><i>calls</i></td><td><i>min. time</i></td><td><i>max. time</i>"
           "</td><td><i>avg. time</i></td></tr>\n");

  // sort methods by access count (in descending order)
  multimap<int, unordered_map<string, tMethodStat>::const_iterator> ops_sorted;
  for (unordered_map<string, tMethodStat>::const_iterator i = access_count_.begin();
       i != access_count_.end();
       ++i) {
    ops_sorted.insert(make_pair(-(i->second.count), i));
  }
  // print method access statistics in a table
  for (multimap<int, unordered_map<string, tMethodStat>::const_iterator>
         ::const_iterator j = ops_sorted.begin();
       j != ops_sorted.end();
       ++j) {
    unordered_map<string, tMethodStat>::const_iterator i = j->second;
    int count = i->second.count;
    html << "<tr align=center><td>" << i->first
         << "</td><td>" << count
         << "</td><td>";
    if (count > 0)
      html << i->second.time_min << "s";
    html << "</td><td>";
    if (count > 0)
      html << i->second.time_max << "s";
    html << "</td><td>";
    if (count > 0)
      html << i->second.time_total / count << "s";
    html << "</td></tr>\n";
  }

  html << "</table><p>\n"
       << "Number of pending calls: " << id_list.size() << "<p>\n";

  if (!id_list.empty()) {
    html << ("List of pending calls:<p>\n"
             "<table border=1>\n"
             "<tr><td><i>seq. #</i></td><td><i>time elapsed</i></td>"
             "<td><i>method</i></td><td><i>protocol</i></td>"
             "<td><i>input</i></td>\n");

    for (int i = 0; i < id_list.size(); i++) {
      unordered_map<int, tPendingMethod *>::const_iterator itr =
        in_progress_.find(id_list[i]);
      ASSERT(itr != in_progress_.end());
      const tPendingMethod *p = itr->second;
      html << "<tr align=center><td>" << p->id
           << "</td>\n<td>" << (curr_time.t() - p->timestamp)
           << "s</td>\n<td>" << p->opname
           << "</td>\n<td>" << (p->is_json ? "json" : "rpc")
           << ("</td>\n<td align=left>"
               "<a href=\"javascript:toggleinput(")
           << i << ")\"><div id=\"toggle" << i
           << "\">show</div></a><div id=\"inputstr" << i
           << "\" style=\"display:none;\"><pre>\n" << p->input_string
           << "\n</pre></div></td></tr>\n";
    }
    html << "</table>\n";
  }

  html << "</body></html>\n";

  return html.str();
}

void RPCServer::PrintProxyConfig() const {
  // sort the list of opcodes
  set<string> opnames = method_collection_->MethodNames<set<string> >();

  // prepare proxy config output
  string my_address = "localhost";

  stringstream proxy_config;
  for (set<string>::const_reverse_iterator i = opnames.rbegin();
       i != opnames.rend(); ++i) {
    const string& opname = *i;
    if (opname.empty() || opname[0] == '.' || opname[0] == '_') continue;

    const ServerMethodForwardCollection::Data* data =
        forward_method_collection_->GetData(opname);
    // check if this opname is being forwarded
    if (data == nullptr) {
      // this opname is not forwarded
      proxy_config << "ProxyPass /" << opname << " $backend/" << opname << "\n";
      /*
      proxy_config << "ProxyPass /" << opname
                   << " http://" << my_address << ":" << portnum_
                   << "/" << opname << "\n";
      */
    }
    else {
      proxy_config << "ProxyPass /" << opname
                   << " balancer://" << data->server_type << "_cluster"
                   << "/" << data->remote_method << "\n";
    }
  }
  // URL redirect method
  proxy_config << "ProxyPass /URL http://" << my_address << ":" << portnum_
               << "/URL\n";

  VLOG(0) << "\n\nSample proxy config for apache2:\n\n"
         << proxy_config.str() << "\n";
}

// construct the root web form for http debug interface
string RPCServer::ConstructRootForm(const unordered_map<string, string>& arg_map) const {
  string mockval = (arg_map.find("mock") == arg_map.end()) ? "" : arg_map.find("mock")->second;
  string js_mockval = "\"" + mockval  + "\";";
  stringstream html;
  html << "<html><head><title>R77 RPC Server</title>"
       << "<link rel=\"icon\" type=\"image/png\" "
       << "href=\"/images/icons/fav-internal.png\" />\n"
       << "<script type=\"text/javascript\">"
       << "<!--\n"
       << "function settarget(seq) {\n"
       << "  var mock = " << js_mockval << "\n"
       << "  var c = document.getElementById(\"targetcheck\" + seq);\n"
       << "  var f = document.getElementById(\"query\" + seq);\n"
       << "  if (c.checked)\n"
       << "    f.target = \"optripjsonresultwin\";\n"
       << "  else\n"
       << "    f.target = \"result\" + seq;\n"
       << "  var m = document.getElementById(\"mock\" + seq);\n"
       << "  m.value = mock;\n"
       << "}\n"
       << "// -->\n"
       << "</script>\n"
       << "</head>\n<body>\n"
       << "<a href=\"/_usage\">Usage report</a>"
       << "<span style='padding-left:100px;'></span>"
       << "<a href=\"/_status\">Process status</a>"
       << "<span style='padding-left:100px;'></span>"
       << "<a href=\"/_param\">View/edit parameters</a>"
       << "<span style='padding-left:100px;'></span>"
       << "<a href=\"/_validate\">Validate</a>"
       << "<span style='padding-left:100px;'></span>"
       << "<a href=\"/_threads\">Thread Stacks</a>"
       << "<p>\n"
       << "This server supports the following " << method_collection_->size()
       << " operation" << (method_collection_->size() > 1 ? "s" : "")
       << ":<p><p>\n";

  // sort handler opnames
  vector<string> opnames = method_collection_->MethodNames<vector<string> >();
    sort(opnames.begin(), opnames.end(), less<string>());

  // provide intra-page links
  for (int count = 0; count < opnames.size(); count++) {
    const string& op = opnames[count];
    html << "<a href=\"#" << op << "\">" << op << "</a> &nbsp; \n";
  }

  html << "<p><p>\n<table>\n";

  for (int count = 0; count < opnames.size(); count++) {
    const string& opname = opnames[count];
    ServerMethodHandlerBase* h = method_collection_->GetHandler(opname);
    ASSERT_NOTNULL(h);

    string sample_input = h->GetSampleInput_JSON();
    int seq = count + 1;

    html << "<tr id=\"" << opname << "\"><form id=\"query" << seq
         << "\" action=\"" << opname
         << "\" method=\"post\" target=\"result" << seq << "\">\n";

    // column 1: opname
    html << "<td><b>" << opname << "</b></td>\n";
    // column 2: input JSON
    html << "<td><textarea name=\"q\" rows=12 cols=55>\n"
         << strutil::EscapeString_HTML(sample_input)
         << "</textarea><br>"
         << "<input type=\"hidden\" name=\"d\" value=\"1\">\n"
         << "<input id=\"mock" << seq << "\" type=\"hidden\" name=\"mock\" value=\"" << mockval  << "\">\n"
         << "</td>\n";
    // column 3: "submit" button
    html << "<td><input type=\"submit\" value=\"Submit\"><br>\n";
    html << "<input type=\"checkbox\" id=\"targetcheck" << seq
         << "\" onclick=\"javascript:settarget(" << seq << ")\">"
         << "<label for=\"targetcheck" << seq << "\"><font size=-1> "
         << "in new<br>&nbsp;&nbsp;&nbsp;&nbsp;window</font></label></td>\n";
    // column 4: result iframe
    html << "<td>Result:<br><iframe name=\"result" << (count + 1)
         << "\" width=390 height=180></iframe></td>\n";

    html << "</form></tr>\n";
  }

  html << "</table>\n<p></body></html>\n";

  return html.str();
}

bool RPCServer::CheckHealth() const {
  bool status = CheckOutgoingNetworkTraffic();
  return status;
}

}  // namespace network
