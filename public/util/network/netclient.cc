//
// Copyright 2007 OpTrip, Inc.
//
// Author: Calvin Yang
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "base/common.h"
#include "util/network/netclient.h"
#include "util/network/dnslookup.h"
#include "util/string/strutil.h"

FLAG_int(netclient_default_timeout, 10000,
         "the default netclient timeout in milliseconds");

FLAG_string(r77_user_agent, "OpTripBot www.optrip.com",
            "The User Agent used to make various network calls.");

FLAG_int(netclient_max_reply_size, 100000000,
         "Default netclient maximum reply size");


NetClient::NetClient() :
  socket_(-1), portnum_(-1), max_reply_size_(gFlag_netclient_max_reply_size),
  timeout_(gFlag_netclient_default_timeout) {
}

NetClient::~NetClient() {
  CloseConnection();
}


// establish connection to server
// (if called again with empty hostname, re-establish connection to the
//  same server)
bool NetClient::EstablishConnection(const string& hostname, int portnum) {
  clear_reply();

  if (!hostname.empty() && portnum >= 0) {
    hostname_ = hostname;
    portnum_ = portnum;
  }

  ASSERT(!hostname_.empty());
  ASSERT_GE(portnum_, 0);

  // make an internet-transmitted, file-i/o-style, protocol-whatever plug
  if ((socket_ = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    LOG(INFO) << strutil::LastSystemError();
    return false;
  }

  // plug it into the listening socket
  //  socketname.sin_family = AF_INET;

  string resolved;
  if (!(DNSUtil::Instance().LookupHost(hostname_, portnum_, &resolved))) {
    LOG(INFO) << "Unknown host: " << hostname_;
    return false;  // unknown host name
  }

  //  socketname.sin_port = htons(portnum_);

  fcntl(socket_, F_SETFL, O_NONBLOCK);

  connect(socket_,
          reinterpret_cast<const struct sockaddr *>(resolved.c_str()),
          resolved.size());

  fd_set fdset;
  struct timeval tv;

  FD_ZERO(&fdset);
  FD_SET(socket_, &fdset);
  tv.tv_sec = timeout_ / 1000;     // timeout for nonblocking connect
  tv.tv_usec = (timeout_ % 1000) * 1000;

  select(socket_ + 1, NULL, &fdset, NULL, &tv);
  if (FD_ISSET(socket_, &fdset)) {
    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(socket_, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error == 0) {
      fcntl(socket_, F_SETFL, fcntl(socket_, F_GETFL) & ~O_NONBLOCK);
      return true;  // successful connection
    }
    else {
      LOG(INFO) << "error connecting to " << hostname_ << ":" << portnum_
             << " -- " << strutil::LastSystemError();
      return false;
    }
  }
  else {
    // timeout
    LOG(INFO) << "connection timed out: " << hostname_ << ":" << portnum_;
    return false;
  }
}


bool NetClient::SendMessage(const string& request) {
  ASSERT_GE(socket_, 0);
  clear_reply();
  int len = request.size();
  int bytes_written = write(socket_, request.c_str(), len);
  return (bytes_written == len);

  /*
  int total_bytes_written = 0;
  while (total_bytes_written < len) {
    int bytes_written = write(socket_,
                              request.c_str() + total_bytes_written,
                              len - total_bytes_written);
    LOG(INFO) << bytes_written;
    if (bytes_written <= 0)
      return false;  // an error has occurred
    total_bytes_written += bytes_written;
  }
  return true;
  */
}


// wait for reply
// return true if a complete reply message is received, or if server closes
// connection
// return false if timeout occurs without response from server, or if an error
// occurs
bool NetClient::WaitForReply() {
  ASSERT_GE(socket_, 0);
  while (!ReplyIsComplete()) {
    tStatus_ status = ReadMoreDataIfAvailable();
    if (status == D_TIMEOUT || status == D_ERROR)
      return false;
    else if (status == D_CLOSED)
      return true;
  }
  return true;
}


// read more data if there is data pending
NetClient::tStatus_ NetClient::ReadMoreDataIfAvailable() {
  ASSERT_GE(socket_, 0);

  // wait for data with a timeout
  int max_fd = socket_ + 1;
  fd_set connections;
  FD_ZERO(&connections);
  FD_SET(socket_, &connections);
  struct timeval time_limit;
  time_limit.tv_sec = timeout_ / 1000;
  time_limit.tv_usec = (timeout_ % 1000) * 1000;
  time_t start_timestamp = time(NULL);
  select(max_fd, &connections, NULL, NULL, &time_limit);
  time_t end_timestamp = time(NULL);

  if (FD_ISSET(socket_, &connections)) {
    // some data has arrived, or server closed connection
    int old_size = reply_.size();
    int ret = strutil::ReadMoreDataFromSocket(socket_, 16384,
                                              max_reply_size_, &reply_);

    if (ret < 0)  // server closed connection without sending any data
      return D_CLOSED;
    else {
      if (old_size < max_reply_size_ &&
          reply_.size() >= max_reply_size_)
        LOG(INFO) << "Warning: reply buffer full.  server="
               << hostname_ << ", port=" << portnum_;
      return D_OK;
    }
  }
  else { // timeout occurs
    LOG(INFO) << "net client timed out (seconds): " << end_timestamp - start_timestamp;
    return D_TIMEOUT;
  }
}

bool NetClient::CloseConnection() {
  clear_reply();
  if (socket_ >= 0) {
    close(socket_);
    socket_ = -1;
    return true;
  }
  else
    return false;
}

