//
// Copyright 2007 OpTrip, Inc.
//
// Author: Calvin Yang
//

//
// SSLClient class is an SSL client supporting simple message I/O.
//

#ifndef _PUBLIC_UTIL_NETWORK_SSLCLIENT_H_
#define _PUBLIC_UTIL_NETWORK_SSLCLIENT_H_

#include "util/network/netclient.h"

struct bio_st;
typedef struct bio_st BIO;
struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;
struct ssl_st;
typedef struct ssl_st SSL;

class SSLClient : public NetClient {
 public:
  SSLClient();
  virtual ~SSLClient();

  // establish connection to server/port specified
  virtual bool EstablishConnection(const string& hostname, int portnum);
  virtual bool EstablishSecureConnection();
  virtual bool EstablishInsecureConnection();

  // send a message to server
  virtual bool SendMessage(const string& request);

  // virtual bool WaitForReply();  -- same as base class

  // close connection to server
  virtual bool CloseConnection();

 protected:
  virtual tStatus_ ReadMoreDataIfAvailable();

  // ssl objects
  BIO *bio_;
  SSL_CTX *ctx_;
  SSL *ssl_;
};

#endif  // _PUBLIC_UTIL_NETWORK_SSLCLIENT_H_
