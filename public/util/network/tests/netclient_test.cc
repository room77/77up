//
// Copyright 2007 OpTrip, Inc.
//

#include "util/network/netclient.h"
#include "util/string/strutil.h"
#include "util/init/main.h"

FLAG_string(host, "www.optrip.com", "host to connect to");
FLAG_int(port, 80, "port to connect to");
FLAG_int(timeout, 10, "connection timeout in seconds");

int init_main() {

  NetClient c;
  c.set_timeout(gFlag_timeout);

  //  c.set_max_reply_size(100);
  if (c.EstablishConnection(gFlag_host, gFlag_port)) {
    c.SendMessage("GET / HTTP/1.1\n"
		  "Host: www.optrip.com\n"
		  "User-Agent: OpNetClient\n"
		  "Connection: Close\n"
		  "Accept: text/html,text/xml,text/plain\n"
		  "\n");
    c.WaitForReply();

    LOG(INFO) << "Reply is:\n" << c.get_reply() << "\n\n";
    LOG(INFO) << "(" << c.get_reply().size() << " bytes received.)\n";
    ASSERT(strutil::IsPrefix("HTTP/1.1 200",
			     c.get_reply().c_str()));

    c.CloseConnection();

    cout << "PASS\n";

    return 0;
  }
  else {
    LOG(INFO) << "Unable to establish connection.\n";
    return 2;
  }
}
