// Copyright 2012 Room77, Inc.
// Author: Nicholas Edelman

#include "base/common.h"
#include "util/network/dnslookup.h"
#include "util/network/util.h"
#include "util/log/remote_log.h"
#include "util/serial/serializer.h"
#include "util/time/timestamp.h"

#include <syslog.h>
#include <sys/time.h>

FLAG_int(msgprefix_len, 6, "the message prefix must be this long");
FLAG_string(crawl_prefix, "cralog", "the crawl log prefix");
FLAG_string(gds_prefix, "gdslog", "the crawl log prefix");
FLAG_string(price_prefix, "prilog", "the price log prefix");
FLAG_string(server_prefix, "serlog", "the crawl log prefix");
FLAG_string(web_prefix, "weblog", "the web log prefix");
FLAG_string(partner_prefix, "parlog", "the partner log prefix");
FLAG_string(badweb_prefix, "badweb", "the badweb log prefix");
FLAG_string(devweb_prefix, "devweb", "the development web log prefix");

namespace util {

  const RemoteLog& RemoteLog::Instance() {
    static RemoteLog logger;
    return logger;
  }

  RemoteLog::~RemoteLog() {
    closelog();
  }

  void RemoteLog::StructuredLog(const string& category,
                                const string& action,
                                const LogMsg& msg,
                                const MsgType& msgtype) const {
    LogMsg log;
    log["category"] = category;
    log["action"] = action;
    log["ip"] = NetworkUtil::MyIP4Address();
    log["host"] = DNSUtil::Instance().MyHostname();
    log["server_time"] = ::serial::Serializer::ToJSON(
        ::util::Timestamp::Now<chrono::milliseconds>());
    log["value"] = ::serial::Serializer::ToJSON(msg);
    Log(::serial::Serializer::ToJSON(log), msgtype);
  }

  void RemoteLog::Log(const string& msg, const MsgType& msgtype) const {
    stringstream ss;
    ss << msgprefix_.find(msgtype)->second << "|" << msg;
    syslog(LOG_INFO | LOG_LOCAL0, "%s", ss.str().c_str());
  }

  //
  // === PRIVATE ===
  //

  RemoteLog::RemoteLog() {
    // We log prices to syslog as facility LOCAL0
    openlog("", LOG_NDELAY, LOG_LOCAL0);
    // map from MsgType to logging string
    msgprefix_[MsgType::CRAWL] = gFlag_crawl_prefix;
    msgprefix_[MsgType::GDS] = gFlag_gds_prefix;
    msgprefix_[MsgType::PRICE] = gFlag_price_prefix;
    msgprefix_[MsgType::SERVER] = gFlag_server_prefix;
    msgprefix_[MsgType::WEB] = gFlag_web_prefix;
    msgprefix_[MsgType::PARTNER] = gFlag_partner_prefix;
    msgprefix_[MsgType::BADWEB] = gFlag_badweb_prefix;
    msgprefix_[MsgType::DEVWEB] = gFlag_devweb_prefix;
    // validate that all the prefixs are long enough
    for (auto& el: msgprefix_) ASSERT(el.second.size() == gFlag_msgprefix_len)
                                  << el.second.size() << " prefix is not "
                                  << gFlag_msgprefix_len << " characters";
  }

} // namespace util
