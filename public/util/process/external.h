#ifndef _PUBLIC_UTIL_PROCESS_EXTERNAL_H_
#define _PUBLIC_UTIL_PROCESS_EXTERNAL_H_

#include "base/common.h"

//
// write input to a temporary file, launch an external process to read
// from input and write to another temporary file; collect the result and
// return
//

namespace External {

  // command: OS command to execute;
  //          %i will be replaced by temporary input file name
  //          %o will be replaced by temporary output file name
  bool Call(const string& command,
            const string& input, string *output, int *status);

}


#endif  // _PUBLIC_UTIL_PROCESS_EXTERNAL_H_
