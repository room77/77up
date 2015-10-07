//
// Copyright 2007 OpTrip, Inc.
//

//
// NetClient class is a raw TCP/IP client and supports simple message I/O.
//

#ifndef _PUBLIC_UTIL_NETWORK_NETCLIENT_H_
#define _PUBLIC_UTIL_NETWORK_NETCLIENT_H_

#include "base/common.h"

extern int gFlag_netclient_default_timeout;
extern string gFlag_r77_user_agent;

class NetClient {
 public:
  NetClient();

  virtual ~NetClient();

  // maximum size of reply message.  default: 100,000,000 bytes
  // -1 means no limit
  inline virtual void set_max_reply_size(int size) { max_reply_size_ = size; }
  // server timeout limit
  inline virtual void set_timeout(int num_seconds) { timeout_ = num_seconds * 1000; }
  // set the connection timeout in ms
  inline virtual void set_timeout_ms(int num_ms) { timeout_ = num_ms; }

  // reply message buffer is stored in member variable reply_
  inline virtual const string& get_reply() const { return reply_; }
  inline virtual void clear_reply() { reply_.clear(); }

  // main API

  // establish connection to server/port specified
  virtual bool EstablishConnection(const string& hostname, int portnum);

  // send a message to server
  virtual bool SendMessage(const string& request);

  // wait for reply
  // return true if a complete reply message is received, or if server closes
  // connection
  // return false if timeout occurs without response from server, or if an
  // error occurs
  virtual bool WaitForReply();

  // close connection to server
  virtual bool CloseConnection();

 protected:
  int socket_;       // fd for the listening socket
  string hostname_;  // remote server name
  int portnum_;      // remote port number

  string reply_;     // reply message buffer
  int max_reply_size_;  // reply message size limit (-1: no limit)
  int timeout_;  // server timeout limit (in milliseconds)

  // helper function -- continue reading more data from server
  typedef enum {
    D_TIMEOUT, D_ERROR, D_CLOSED, D_OK
  } tStatus_;
  virtual tStatus_ ReadMoreDataIfAvailable();

  // is the incoming reply data complete?
  //     (return true if the request is complete, false if not or unknown.)
  //   default: no (request is complete only if caller closes connection)
  //   override if there is a way to determine whether the request is complete
  virtual bool ReplyIsComplete() const { return false; }
};

#endif  // _PUBLIC_UTIL_NETWORK_NETCLIENT_H_
