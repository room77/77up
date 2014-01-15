//
// Copyright 2007 OpTrip, Inc.
//

//
// NetServer class is a raw TCP/IP server and supports simple message I/O.
//

#ifndef _PUBLIC_UTIL_NETWORK_NETSERVER_H_
#define _PUBLIC_UTIL_NETWORK_NETSERVER_H_

#include <cstdio>
#include <netinet/in.h>
#include <unistd.h>

#include "base/common.h"
#include "util/thread/thread_pool.h"

class NetServer;
struct tConnectionInfo {
  int ear;
  struct sockaddr_in caller_id;
  NetServer *server;
};

class NetServer {
 public:
  NetServer();
  virtual ~NetServer() {}

  inline int get_portnum() const { return portnum_; }
  inline int get_max_request_size() const { return max_request_size_; }
  inline int get_timeout() const { return timeout_; }
  int get_max_connections() const { return max_connections_; }

  // port number of the server
  // if negative, system will pick any port
  inline void set_portnum(int portnum) { portnum_ = portnum; }

  // maximum size of incoming request message.  default: 100,000,000 bytes
  // -1 means no limit
  inline void set_max_request_size(int size) {
    max_request_size_ = size;
  }

  // server timeout limit.
  // -1 means no timeout
  inline void set_timeout(int num_seconds) { timeout_ = num_seconds; }

  // maximum number of concurrent connections
  inline void set_max_connections(int num) {
    max_connections_ = num;
  }

  virtual void StartServer();

  //
  // The following functions may be called from multiple threads.
  // They must be thread-safe.
  //

  // is the incoming request complete?
  //     (return true if the request is complete, false if not or unknown.)
  //   default: no (request is complete only if caller closes connection)
  //   override if there is a way to determine whether the request is complete
  // if request is complete, request_size should be set to the length of
  // the request string
  virtual bool RequestIsComplete(const string& r,
                                 unsigned int *request_size) const {
    return false;
  }

  // is there only one request per connection?
  //   default: true.  (connection will be closed after a request is processed)
  //   Override to false if multiple requests are allowed per connection (in
  //       which case RequestIsComplete() must be implemented, and connection
  //       will be closed by timeout or by client).
  virtual bool OneRequestPerConnection() const { return true; }

  // call this function when a connection is closed (default behavior: no-op.)
  virtual void OnCloseConnection(const tConnectionInfo *connection) const {};

  // construct an appropriate message indicating the server is busy
  // (default: no message; just close the connection)
  virtual string ServerBusyMessage() const { return ""; };

  // process a request after it has been completely received
  virtual string ProcessRequest(const string& request,
                                bool *keep_alive,
                                const tConnectionInfo *connection) = 0;

  inline void IncrementConnectionCounter() { ++num_connections_; }
  inline void DecrementConnectionCounter() { --num_connections_; }

  inline void IncrementPendingRequestCounter() { ++num_pending_requests_; }
  inline void DecrementPendingRequestCounter() { --num_pending_requests_; }

  void set_prepare_shutdown(bool status = true) {
    prepare_shutdown_ = status;
  }
  inline bool PreparingShutdown() const { return prepare_shutdown_; }

  void set_return_on_shutdown(bool status = true) {
    return_on_shutdown_ = status;
  }

  // prepare server before starting.
  virtual void PrepareServer();

 protected:

  inline static void WriteMessage(int ear, const string& msg) {
    int bytes_written = write(ear, msg.c_str(), msg.size());
    if (bytes_written < msg.size())
      VLOG(2) << "Only " << bytes_written << " out of " << msg.size()
             << " bytes are written successfully.";
  }

  // connection handler -- run as a separate thread
  static void NewConnection(tConnectionInfo* info);

  // log caller IP and additional info
  void LogCaller(const struct sockaddr_in& caller,
                 const string& log_info);

  void CloseMainSocket() { int s = socket_; socket_ = -1; close(s); }

  // Returns the common thread pool.
  util::threading::ThreadPool* thread_pool() { return pool_.get(); }

  int portnum_;  // port number of this server
  int max_request_size_;  // max size of incoming request
  int timeout_;  // timeout in seconds (-1: no limit)
  int max_connections_;  // max number of concurrent connections to server
  std::atomic_int num_connections_, num_pending_requests_;
  bool prepare_shutdown_, shutdown_loop_;
  bool return_on_shutdown_ = false;
  int socket_;  // main listening socket

  // Thread pool to handle requests.
  unique_ptr<util::threading::ThreadPool> pool_;
};

#endif  // _PUBLIC_UTIL_NETWORK_NETSERVER_H_
