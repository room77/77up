// Copyright 2007 Optrip.com. All Rights Reserved.
// Author: yangc@room77.com (Calvin Yang)

// HttpClient and HttpsClient implement http GET/POST commands
// They are both instances of HttpBase

#ifndef _PUBLIC_UTIL_NETWORK_HTTPCLIENT_H_
#define _PUBLIC_UTIL_NETWORK_HTTPCLIENT_H_

#include "util/network/sslclient.h"
#include "util/network/httputil.h"
#include "util/string/strutil.h"

template <class NetBase>
class HttpBase : public NetBase {
 public:
  HttpBase() : user_agent_(gFlag_r77_user_agent) {}

  explicit HttpBase(const HttpBase& s)
      : user_agent_(s.user_agent_), auth_string_(s.auth_string_),
        extra_header_(s.extra_header_) {
    this->timeout_ = s.timeout_;
  }

  virtual ~HttpBase() {}

  inline const string& user_agent() const { return user_agent_; }
  inline void set_user_agent(const string& agent) { user_agent_ = agent; }

  inline void BasicAuthentication(const string& username,
                                  const string& password) {
    auth_string_ = strutil::EncodeString_Base64(username + ":" + password);
  }

  inline void AddHeader(const string& header) {
    for (int i = 0; i < header.size(); i++) {
      ASSERT(header[i] >= ' ')
        << "Invalid character in header (char " << (i + 1) << "): " << header;
    }

    if (header.compare(0, 13, "Content-Type:") == 0) {
      // Make a note - we've changed content type
      VLOG(4) << "Changed content type with addheader";
      changed_content_type_ = true;
    }
    extra_header_ += (header + "\r\n");
  }

  virtual bool Get(const string& path, int *status_code, string *reply,
                   HttpUtil::tHeaderType *header = NULL);

  virtual bool Post(const string& path, const string& message,
		    int *status_code, string *reply,
                    HttpUtil::tHeaderType *header = NULL);

  virtual bool Put(const string& path, const string& message,
                   int *status_code, string *reply,
                   HttpUtil::tHeaderType *header = NULL);

  // open a connection, perform a "GET" command and close the connection
  bool HttpGet(const string& host, int port, const string& path,
               int *status_code, string *reply,
               HttpUtil::tHeaderType *header = NULL) {
    bool success = false;
    *status_code = -1;
    reply->clear();
    if (NetBase::EstablishConnection(host, port)) {
      if (Get(path, status_code, reply, header))
        success = true;
      NetBase::CloseConnection();
    }
    return success;
  }

  // open a connection, perform a "POST" command and close the connection
  bool HttpPost(const string& host, int port,
                const string& path, const string& message,
                int *status_code, string *reply,
                HttpUtil::tHeaderType *header = NULL) {
    bool success = false;
    *status_code = -1;
    reply->clear();
    if (NetBase::EstablishConnection(host, port)) {
      if (Post(path, message, status_code, reply, header))
        success = true;
      NetBase::CloseConnection();
    }
    return success;
  }

  // open a connection, perform a "PUT" command and close the connection
  bool HttpPut(const string& host, int port,
               const string& path, const string& message,
               int *status_code, string *reply,
               HttpUtil::tHeaderType *header = NULL) {
    bool success = false;
    *status_code = -1;
    reply->clear();
    if (NetBase::EstablishConnection(host, port)) {
      if (Put(path, message, status_code, reply, header))
        success = true;
      NetBase::CloseConnection();
    }
    return success;
  }

 protected:
  string user_agent_;
  string auth_string_;
  string extra_header_;
  bool changed_content_type_ = false;

  /*
  // is the incoming reply data complete?
  //     (return true if the request is complete, false if not or unknown.)
  //
  // not implemented -- as of now, all requests should be sent with
  // "Connection: Close".  In order to support Keep-Alive connections,
  // this method needs to be implemented to examine Content-Length and,
  // in case Transfer-Encoding is chunked, content body as well.
  //
  virtual bool ReplyIsComplete() const {
    return false;
  }

  */
};

template <class NetBase>
bool HttpBase<NetBase>::Put(const string& path, const string& message,
                             int *status_code, string *reply,
                             HttpUtil::tHeaderType *header) {
  reply->clear();
  *status_code = -1;

  stringstream msg(stringstream::out);
  msg << "PUT " << path << " HTTP/1.1\r\n"
      << "User-Agent: " << user_agent_ << "\r\n"
      << "Accept-Encoding: gzip\r\n"
      << "Content-Length: " << message.size() << "\r\n";
  if (!changed_content_type_)
      msg << "Content-Type: text/xml; charset=utf-8\r\n";

  if (!(auth_string_.empty()))
    msg << "Authorization: Basic " << auth_string_ << "\r\n";

  msg << "Connection: Close\r\n"
      << "Host: " << NetBase::hostname_ << "\r\n"
      << extra_header_
      << "\r\n"
      << message;

  VLOG(3) << "Sending PUT message:\n" << msg.str();
  if (!(NetBase::SendMessage(msg.str())))
    return false;

  if (!(NetBase::WaitForReply()))
    return false;

  bool success = HttpUtil::ParseHttpResponse(NetBase::reply_,
                                             status_code, header, reply);
  NetBase::clear_reply();

  return success;
}

template <class NetBase>
bool HttpBase<NetBase>::Post(const string& path, const string& message,
                             int *status_code, string *reply,
                             HttpUtil::tHeaderType *header) {
  reply->clear();
  *status_code = -1;

  stringstream msg(stringstream::out);
  msg << "POST " << path << " HTTP/1.1\r\n"
      << "User-Agent: " << user_agent_ << "\r\n"
      << "Accept-Encoding: gzip\r\n"
      << "Content-Length: " << message.size() << "\r\n";
  if (!changed_content_type_)
      msg << "Content-Type: text/xml; charset=utf-8\r\n";

  if (!(auth_string_.empty()))
    msg << "Authorization: Basic " << auth_string_ << "\r\n";

  msg << "Connection: Close\r\n"
      << "Host: " << NetBase::hostname_ << "\r\n"
      << extra_header_
      << "\r\n"
      << message;

  VLOG(3) << "Sending POST message:\n" << msg.str();
  if (!(NetBase::SendMessage(msg.str())))
    return false;

  if (!(NetBase::WaitForReply()))
    return false;

  bool success = HttpUtil::ParseHttpResponse(NetBase::reply_,
                                             status_code, header, reply);
  NetBase::clear_reply();

  return success;
}


template <class NetBase>
bool HttpBase<NetBase>::Get(const string& path,
                            int *status_code, string *reply,
                            HttpUtil::tHeaderType *header) {
  reply->clear();
  *status_code = -1;

  stringstream msg(stringstream::out);
  msg << "GET " << path << " HTTP/1.1\r\n"
      << "User-Agent: " << user_agent_ << "\r\n"
      << "Accept-Encoding: gzip\r\n";

  if (!(auth_string_.empty()))
    msg << "Authorization: Basic " << auth_string_ << "\r\n";

  msg << "Connection: Close\r\n"
      << "Host: " << NetBase::hostname_ << "\r\n"
      << extra_header_
      << "\r\n";

  VLOG(4) << "Sending GET message:\n" << msg.str();
  if (!(NetBase::SendMessage(msg.str())))
    return false;

  if (!(NetBase::WaitForReply()))
    return false;

  VLOG(4) << "Reply is:\n" << NetBase::reply_;

  bool success = HttpUtil::ParseHttpResponse(NetBase::reply_,
                                             status_code, header, reply);
  NetBase::clear_reply();

  return success;
}

typedef HttpBase<NetClient> HttpClient;
typedef HttpBase<SSLClient> HttpsClient;

#endif  // _PUBLIC_UTIL_NETWORK_HTTPCLIENT_H_
