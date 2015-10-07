// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#ifndef _PUBLIC_UTIL_INIT_MAIN_H_
#define _PUBLIC_UTIL_INIT_MAIN_H_

namespace r77 {

// This must be called by all main() functions to init all
// modules correctly.
// If you want to have your own main (e.g. if you want to process/modify argc,
// argv before we pass them to ArgsInit to parse) make sure you call this
// within your main().
void R77Init(int argc, char** argv);

// This must be called at the end of all main() functions to shutdown modules
// correctly. It is called automatically if you use init_main() instead.
void R77Shutdown();

}  // namespace r77


#endif  // _PUBLIC_UTIL_INIT_MAIN_H_
