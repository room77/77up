#include "util/network/netserver.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>

#include <chrono>
#include <thread>
#include <fstream>

#include "util/string/strutil.h"

// Definitions of these functions are hijacked by macros in opt mode, which
// causes compiler errors with gcc 4.9. Undefine them for now to avoid issues.
// TODO(oztekin,pramodg,everyone): Remove these undefs when glibc is fixed.
#undef ntohs
#undef htons

FLAG_int(use_thread_pool, false,
         "Use thread pool for the server.");

FLAG_int(server_thread_pool_size, 512,
         "The number of default threads created for the server.");

FLAG_int(netserver_max_connections, 1024,
         "The maximum number of connections allowed by netserver.");


NetServer::NetServer() :
    portnum_(-1), max_request_size_(100000000), timeout_(60),
    max_connections_(gFlag_netserver_max_connections),
    num_connections_(0), num_pending_requests_(0), prepare_shutdown_(false),
    shutdown_loop_(false), socket_(-1) {}

//
// prepare the server (open a socket, etc.)
//
void NetServer::PrepareServer() {
  //
  //  open a net socket, using stream (file-style i/o)
  //  mode, with protocol irrelevant ( == 0 )
  //
  ASSERT_EQ(socket_, -1);
  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  ASSERT(socket_ >= 0) << strutil::LastSystemError();

  struct sockaddr_in sock_addr;	        // sytem's location of the socket
  //
  //  register the main socket
  //
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = INADDR_ANY;   // not choosy about who calls

  // if portnum_ <= 0, bind to any port
  // if portnum_ > 0,  bind to given port
  sock_addr.sin_port = (portnum_ <= 0 ? 0 : htons(portnum_));

  //
  // allow reuse of port immediately after closing (for easy debugging)
  //
  int reuseaddr = 1;
  setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

  ASSERT(::bind(socket_, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) >= 0)
    << "Unable to bind port -- " << strutil::LastSystemError();

  //
  //  get port number
  //
  socklen_t sockaddr_in_length = sizeof(struct sockaddr_in);
  ASSERT(getsockname(socket_, (struct sockaddr *) &sock_addr,
                     &sockaddr_in_length) >= 0)
                       << "Cannot get port number for socket " << socket_
                       << " -- " << strutil::LastSystemError();

  // update portnum_ in case it changed
  portnum_ = ntohs(sock_addr.sin_port);
  VLOG(0) << "Server started on port " << portnum_;
  VLOG(0) << "-----------------------";

  // put an ear to the socket, listening for a knock-knock-knocking
  listen(socket_, 1);                      // 1: only one queue slot

  // close(0); close(1); close(2);    // close stdin, stdout and stderr

  if (gFlag_use_thread_pool) {
    pool_.reset(new util::threading::ThreadPool(gFlag_server_thread_pool_size));
    ASSERT_NOTNULL(pool_);
  }
}

// log the caller IP and additional info
void NetServer::LogCaller(const struct sockaddr_in& caller,
                          const string& log_info) {
  string s_IPaddr;

  s_IPaddr = inet_ntoa(caller.sin_addr);   // IP address (string)
  // in_addr_t l_IPaddr = inet_addr(s_IPaddr.c_str());  // IP address (number)

  // // get caller's name on the network  (warning: may be slow!)
  //
  // char *s_netname;
  // if ((h = gethostbyaddr((char *) &l_IPaddr, sizeof(l_IPaddr), AF_INET))
  //     != NULL)
  //   s_netname = h->h_name;    // name on network

  // LOG(INFO) << setiosflags(ios::left) << setw(15) << setfill(' ') << s_IPaddr
  //        << "  " << log_info;
  LOG(INFO) << s_IPaddr
         << (s_IPaddr.size() < 15 ? string(15 - s_IPaddr.size(), ' ') : "")
         << "  " << log_info;
}

//
// start the server
//
void NetServer::StartServer() {
  if (socket_ < 0)
    PrepareServer();

  while (1) {
    if (prepare_shutdown_ || socket_ < 0) break;

    // wait for data arriving on main socket
    int max_fd = socket_ + 1;

    fd_set connections;
    FD_ZERO(&connections);
    FD_SET(socket_, &connections);

    // wait for data with a timeout of 1 second -- i.e., check for _shutdown
    // status every 1 second
    struct timeval time_limit;
    time_limit.tv_sec = 1;
    time_limit.tv_usec = 0;
    struct timeval *timeout_ptr = &time_limit;

    // wait for data with timeout
    int num_active = select(max_fd, &connections, NULL, NULL, timeout_ptr);
    if (prepare_shutdown_) break;

    if (num_active < 0) {
      VLOG(2) << "Warning: select() returns error: "
             << strutil::LastSystemError();
    }
    else if (FD_ISSET(socket_, &connections)) {  // a new connection request
      struct sockaddr_in caller;     // id of foreign calling process
      socklen_t fromlen = sizeof(struct sockaddr_in);

      // ear will be a temporary (non-reusable) socket different from socket_
      int ear = accept(socket_, (struct sockaddr *)&caller, &fromlen);
      ASSERT(ear >= 0) << strutil::LastSystemError();

      if (num_connections_ >= max_connections_ || prepare_shutdown_) {
        // we have already reached the maximum number of concurrent connections
        // or we are about to shut down the server
        if (prepare_shutdown_)
          LOG(INFO) << "Connection declined due to server shutdown";
        else {
          LOG(INFO) << "Connection declined due to max_connections. "
                 << "connections: " << num_connections_.load()
                 << ", max_connections: " << max_connections_;
        }
        WriteMessage(ear, ServerBusyMessage());
        close(ear);
        // nobody knows about this connection yet, so we don't need to call
        // OnCloseConnection().
      }
      else {
        // increment the connection counter
        IncrementConnectionCounter();

        unique_ptr<tConnectionInfo> data(new tConnectionInfo);
        data->ear = ear;
        data->caller_id = caller;
        data->server = this;
        if (pool_ != nullptr)
          pool_->Add(std::bind(&NetServer::NewConnection, data.release()));
        else
          thread(&NetServer::NewConnection, data.release()).detach();
      }
    }

    /*
    // user requested shutdown and server is idle now -- shutdown now
    if (num_pending_requests_ <= 0 && prepare_shutdown_) break;
    */
  }

  if (pool_ != nullptr) {
    // Wait for the thread pool to finish (for at most 30 seconds).
    pool_->WaitWithTimeout(get_timeout() * 1000);
    pool_.reset(nullptr);
  }

  if (return_on_shutdown_) return;

  LOG(INFO) << "Shutting down server upon request.";
  // Allow sometime for detached threads to finish.
  for (int i = get_timeout(); i > 0; --i) {
    this_thread::sleep_for(chrono::seconds(1));
    ifstream file("/proc/self/status");
    string tok;
    for (file >> tok; file.good() && tok != "Threads:"; file >> tok) ;
    file >> tok;
    stringstream ss(tok);
    int num_threads;
    ss >> num_threads;
    int num_pending_req = num_pending_requests_;
    LOG(INFO) << "Pending requests: " << num_pending_req
           << ", pending threads: " << num_threads - 2
           << ", forced shutdown in " << i << "s";
    if (num_pending_requests_ <= 0 || num_threads <= 2) break;
  }

  ShutdownSystem(shutdown_loop_ ? 100 : 15);  // shut down server
}

// connection handler -- run as a separate thread
void NetServer::NewConnection(tConnectionInfo* info) {
  unique_ptr<tConnectionInfo> ad(info);
  string request;

  int ear = info->ear;
  NetServer *server = info->server;
  bool connection_active = true;
  int select_error_count = 0;

  while (connection_active) {
    int max_fd = ear + 1;
    fd_set connections;
    FD_ZERO(&connections);
    FD_SET(ear, &connections);

    // wait for data with a timeout
    struct timeval time_limit;
    time_limit.tv_sec = server->get_timeout();
    time_limit.tv_usec = 0;

    struct timeval *timeout_ptr =
      (server->get_timeout() > 0 ? &time_limit : NULL);

    int num_active = select(max_fd, &connections, NULL, NULL, timeout_ptr);

    if (num_active < 0) {
      VLOG(2) << "Warning: select() returns error: "
             << strutil::LastSystemError();
      select_error_count++;
      if (select_error_count > 5)
        connection_active = false;
    }
    else if (FD_ISSET(ear, &connections)) {
      // more data has been received

      int ret = strutil::ReadMoreDataFromSocket(ear, 16384,
                                                server->get_max_request_size(),
                                                &request);
      if (ret < 0) {
        // client closed connection
        VLOG(3) << "Client closed connection!";
        if (!(request.empty())) {
          // process the request first
          bool keep_alive;
          server->IncrementPendingRequestCounter();
          WriteMessage(ear,
                       server->ProcessRequest(request, &keep_alive, info));
          server->DecrementPendingRequestCounter();
          request.clear();
        }

        connection_active = false;
      }
      else if (ret == 0) {
        // client request message exceeds length limit
        WriteMessage(ear, "Request message too long\n");
        // terminate the connection
        connection_active = false;
      }
      else {
        // data has been received normally
        unsigned int request_len;
        if (server->RequestIsComplete(request, &request_len)) {
          // A complete request has been received.  Let's process it.
          ASSERT(request_len <= request.size())
            << "Error in RequestIsComplete(): incorrect length ("
            << request_len << ") returned; string size = " << request.size();
          bool keep_alive;
          server->IncrementPendingRequestCounter();
          WriteMessage(ear,
                       server->ProcessRequest(request.substr(0, request_len),
                                              &keep_alive, info));
          server->DecrementPendingRequestCounter();

          // The request has been processed.  Remove it from the string.
          request = request.substr(request_len);

          // close the connection if only one request is allowed per connection
          // (either by client request, or by server configuration)
          if (!keep_alive || server->OneRequestPerConnection())
            connection_active = false;
        } else {
          VLOG(4) << "Incomplete request: " << request.size();
        }
      }
    }
    else {
      // timeout occurred
      if (server->get_timeout() > 0) {
        VLOG(3) << "closing client connection after timeout.";
        connection_active = false;
      }
    }

    if (server->PreparingShutdown())
      break;
  }

  // connection has been terminated (by either client or server)
  close(ear);
  server->OnCloseConnection(info);

  // decrement the connection counter
  server->DecrementConnectionCounter();
}

