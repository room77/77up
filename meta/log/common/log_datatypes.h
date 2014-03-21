// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_LOG_DATATYPES_H_
#define _PUBLIC_META_LOG_COMMON_LOG_DATATYPES_H_

#include "base/defs.h"
#include "util/serial/serializer.h"
#include "util/serial/types/arbit_blob.h"


namespace logging {

// corresponds to the raw request that comes from js
struct tLogRawRequest {
  // The URL of the page from which the log request was made.
  string url;

  // the time of the request(on the client) and not when the event happened
  fixedint<uint64_t> sent_time = 0;

  // contains a list of objects because logs are accumulated and sent periodically
  vector<serial::types::ArbitBlob> logs;

  // The CGI params for the URL. In the new angular routing, the CGI params are logged directly
   // and not with the URL.
   unordered_map<string, string> cgi_params;

  SERIALIZE(DEFAULT_CUSTOM / url*1 / sent_time*2 / logs*3 / cgi_params*4);
};

// this interface struct represents the information that is to be
// filled by the server for every request
struct tLogRequestInterface {
  // 0 or "" if not logged in, user_id of the user when logged in.
  string user_id;
  string session_id;  // cookie based session id

  // this is a duplicated variable for consistency with the old logs and better naming in new ones
  fixedint<uint64_t> created = 0;  // (deprecated-use received_time)
  fixedint<uint64_t> received_time = 0;  // Time(microsec) when the log request was received by the server.

  // The URL of the page from which the log request was made.
  string url;

  // The CGI params for the URL. In the new angular routing, the CGI params are logged directly
  // and not with the URL.
  unordered_map<string, string> cgi_params;

  // The IP of the user.
  string user_ip;

  // this is a duplicated variable for consistency with the old logs and better naming in new ones
  string agent;  //  (deprecated-use user_agent)
  string user_agent;  // the platform info from which the request is made

  string lang;
  string host;  // The hostname of the caller. e.g. "www.room77.com"
  string channel;   // web, mobile-web, tablet-app-windows etc.
  bool is_mobile = false;   // stores whether channel is mobile or not
  string user_country;

  // this method fills in the structure with the fields from the argument structure
  void MergeFrom(const tLogRequestInterface& req) {
    *this = req;
  }

  SERIALIZE(DEFAULT_CUSTOM / user_id*1 / session_id*2 / created*3 / received_time*4 / url*5 /
            user_ip*6 / agent*7 / user_agent*8 / lang*9 / host*10 / channel*11 / is_mobile*12 /
            user_country*13 / cgi_params*14);
};

// parsed request populated with additional information. This info is populated for each
// object from the logs field of the raw request.
struct tLogElement : public tLogRequestInterface {
  string id;  // request id
  string pid;  // previous page id
  string nid;  // next page id
  string category;  // category of the log ("New Visit", "Hotel Search", "Sort" etc.)
  string action;  // action of the log ("Dated Search", "Distance Click", "Price" etc.)
  serial::types::ArbitBlob value;  // contains json object describing the request

  // this is a duplicated variable for consistency with the old logs and better naming in new ones
  // it is the actual client time that we think user should have had (correcting for things like
  // clock skews, etc) - it is an estimate of the server time at the time when the event on the
  // client happened (received_time - sent_time + user_time)
  fixedint<uint64_t> server_time = 0;  // (deprecated: use corrected_user_time)
  fixedint<uint64_t> corrected_user_time = 0;  // microsec

  // time(microsec) (on the client) when a particular event happened - note that this is different
  // from sent_time because logs are processed in batches
  fixedint<uint64_t> user_time = 0;

  // specifies the version of the logs (added with the new website logging 11/2013)
  int log_version = 1;

  struct tLogProcessingInfo {
    string read_filename;  // filename of the file from where the log was read
  };

  // This field is not serialized. It is not contained in json and is used currently to add info
  // about the filename from which the element was read.
  tLogProcessingInfo element_add_info;

  SERIALIZE(DEFAULT_CUSTOM / user_id*1 / session_id*2 / created*3 / received_time*4 / url*5 /
            user_ip*6 / agent*7 / user_agent*8 / lang*9 / host*10 / channel*11 / is_mobile*12 /
            user_country*13 / id*14 / pid*15 / nid*16 / category*17 / action*18 / value*19 /
            server_time*20 / corrected_user_time*21 / user_time*22 / cgi_params*23 / log_version*24);
};

}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_LOG_DATATYPES_H_
