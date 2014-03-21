// Copyright 2012 Room77, Inc.
// Author: Nicholas Edelman

#ifndef _PUBLIC_UTIL_LOG_REMOTE_LOG_H_
#define _PUBLIC_UTIL_LOG_REMOTE_LOG_H_

#include <map>
#include <unordered_map>

namespace util {

class RemoteLog {
 public:
  // TODO(edelman) switch to map once pramod fixes the serialization
  //  of map to NOT be vector of pairs
  typedef unordered_map<string, string> LogMsg;

  // when add a new message type, you need to define the prefix
  // in the RemoteLog constructor
  // AND add handlers to rsyslog config client and server files
  // for this new message type
  // AND add type in script/rsyslog/sync-to-s3.sh:logtypes and
  // create a new s3 bucket
  enum class MsgType {CRAWL, GDS, PRICE, SERVER, WEB, PARTNER, BADWEB, DEVWEB};
  static const RemoteLog& Instance();
  ~RemoteLog();
  // log a structure message
  // @param msg - use ::serial:Serializer::ToJSON to convert
  //   the values to strings
  // @param msgtype - specify the location to dump these logs
  void StructuredLog(const string& category,
                     const string& action,
                     const LogMsg& msg,
                     const MsgType& msgtype = MsgType::SERVER) const;
  // log an arbitrary string message
  void Log(const string& msg, const MsgType& msgtype = MsgType::SERVER) const;
 private:
  RemoteLog();
  // the msg prefix map from type to the prefix
  map<MsgType, string> msgprefix_;
};

} // namespace util

#endif  // _PUBLIC_UTIL_LOG_REMOTE_LOG_H_
