// for debugging only

//
// use google heap profiler to check for memory leaks in SSLClient
//

// @flags: -ltcmalloc

#include "util/network/sslclient.h"
#include "util/string/strutil.h"
#include "util/init/main.h"
#include <google/heap-profiler.h>

FLAG_string(profile_prefix, "/localdisk/profile",
            "prefix for heap profile dump files");

void TestSSL() {
  SSLClient c;
  //  c.set_max_reply_size(100);
  ASSERT(c.EstablishConnection("mail.google.com", 443))
    << "Unable to establish connection";

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
}

int init_main() {

  HeapProfilerStart(gFlag_profile_prefix.c_str());

  for (int i = 0; i < 5; i++) {
    TestSSL();
    HeapProfilerDump(gFlag_profile_prefix.c_str());
  }

  HeapProfilerStop();

  LOG(INFO) << "\n\nUse the following command to check for memory leaks:\n\n"
         << "pprof --gv " << GetExecutableName() << " --base=" << gFlag_profile_prefix
         << ".0001.heap " << gFlag_profile_prefix << ".0005.heap\n";

  return 0;
}
