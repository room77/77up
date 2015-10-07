// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_META_LOG_OFFLINE_USER_LOG_ELEMENT_H_
#define _PUBLIC_META_LOG_OFFLINE_USER_LOG_ELEMENT_H_

#include "base/defs.h"

#include "meta/log/common/log_datatypes.h"
#include "util/serial/serializer.h"

namespace logging {

// The structure that is read back from the user logs.
// Ideally this should be the same as tLogElement. However, as things get deleted, renamed,
// etc. in tLogElement, some fields may need fixing so as to allow backward compatibility in
// log parsing.
struct tUserLogReadBackElement : public tLogElement {
  // This field was deprecated in JULY 2013 in favor of URL.
  // It should be safe to remove after JULY 2014.
  string referer_url;

  // User up.
  // This field was deprecated in SEP 2013 in favor of URL.
  // It should be safe to remove after SEP 2014.
  string ip;

  bool DeserializationCallback();

  SERIALIZE(DEFAULT_CUSTOM / user_id*1 / session_id*2 / created*3 / received_time*4 / url*5 /
            user_ip*6 / agent*7 / user_agent*8 / lang*9 / host*10 / channel*11 / is_mobile*12 /
            user_country*13 / id*14 / pid*15 / nid*16 / category*17 / action*18 / value*19 /
            server_time*20 / corrected_user_time*21 / user_time*22 / cgi_params*23 /
            // Leave a few numbers in between in case a few more fields get added.
            referer_url*30 / ip*31);
};

}  // namespace logging


#endif  // _PUBLIC_META_LOG_OFFLINE_USER_LOG_ELEMENT_H_
