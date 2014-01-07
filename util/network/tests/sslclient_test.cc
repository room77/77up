//
// Copyright 2007 OpTrip, Inc.
//

#include "base/logging.h"
#include "util/init/main.h"
#include "util/string/strutil.h"
#include "util/network/sslclient.h"

int init_main() {

  SSLClient c;
  //  c.set_max_reply_size(100);
  if (c.EstablishConnection("mail.google.com", 443)) {
    c.SendMessage("GET / HTTP/1.1\n"
		  "Host: mail.google.com\n"
		  "Connection: Close\n"
		  "User-Agent: OpNetClient\n"
		  "Accept: text/html,text/xml,text/plain\n"
		  "\n");
    c.WaitForReply();

    LOG(INFO) << "Reply is:\n" << c.get_reply() << "\n\n";
    LOG(INFO) << "(" << c.get_reply().size() << " bytes received.)\n";
    ASSERT(strutil::IsPrefix("HTTP/1.1", c.get_reply().c_str()));

    c.CloseConnection();

    cout << "PASS\n";

    return 0;
  }
  else {
    LOG(INFO) << "Unable to establish connection.\n";
    return 2;
  }
}
