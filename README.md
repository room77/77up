# Room 77 C++ core libraries and utilities
This is a collection of the core C++ libraries and utilities used at Room 77 Inc

## Requirements
You will need a recent compiler with decent C++11 support (e.g. gcc 4.8+, clang 3.5+). Most code should work with gcc. Some are tested with clang as well.

## Building

#### Flash
We created the `flash` build system for easily building libraries, binaries and tests. The `RULES` files found in many directories describe the dependencies, compile flags, and source files for each rule.

To use, you will need to clone the [py77](https://github.com/room77/py77) repo. Once cloned, `flash` can be used right out of the box since it has no dependencies. For convenience, you can add `(py77 repo root)/bin` to your `PATH` for easy access to flash.

To build a library, call `flash build <RULE_NAME>`. For example:

    flash build public/util/counter/metrics/all_metrics

Output files are stored in the `e` directory at the repo root.

To run a binary, call `flash run <RULE_NAME>`. For example:

    flash run public/util/thread/task_server_example.cc
    
Alternatively, you can first `flash build` and then run the newly built binary directly from the `e` directory. For example:

    flash build public/util/thread/task_server_example.cc && e/task_server_example

#### auto_build
We provided a subset of Uygar's ruleless build system (that he plans to open source in the future when more mature). All the code, provided that you have the necessary libraries, compiler versions and tools installed, should compile with that. It is known to work with g++ 4.8 and gnu make. Just call make at the root directory to build everything that can be built with the available libraries. 

See `./auto_build --help` for example usage and options.

## Testing / Mocking
A good portion of the tests use googletest / googlemock which are not included as part of this package. We added convenience scripts under third_party/google/* you can use to fetch and incorporate them if you desire. Some of the tests and a small subset of libraries will not compile without them.

The steps for running tests with `flash` are the same as running binaries above, except you call `flash test`. For example:

    flash test public/util/cache/proxy_cache_test.cc

or

    flash build public/util/cache/proxy_cache_test.cc && e/proxy_cache_test
  
## License
The public directory contains open sourced code under a permissive MIT license. Some of the code is copyrighted by Optrip Inc, some by Room 77 Inc, and some by individual authors. Irrespective of the copyright owner, the intention (except third_party) is to publish the code under a permissive MIT or BSD like open source license.

The third_party directory contains open source code used by some of the Room 77 codebase. They may have their own licenses which may be more restrictive than the MIT license (e.g. GPL). The rest of the documentation, unless explicitly stated otherwise, refers to code open sourced by Room 77.

You can use, modify, redistribute the software as you see fit. The software is made available as is without any guarantees or warranties. Optrip, Room 77 or individual authors are not responsible for any damages you may incur as a result of using this code or a subset of it. It is your responsibility to check the code for correctness and suitability before use.

## Thoughts
The software is shared with the hopes that it will be useful to others. If you find any bugs, make improvements, or just want to let us know your experience with it, we would love to hear from you.

Some of the libraries / components are fairly independent and can easily be compiled or added to other codebases. Some of the components have more dependencies that are not as trivial to decouple. We tried to streamline the dependencies for most useful components when practical.

Some of the code depend on external libraries (e.g. geoip, icu, zlib, leveldb, etc.). In order to compile and use them, you would need to install the appropriate libraries. In most Linux distributions, you may already have packages for them.

The quality of the code from our perspective varies from fine to good to great. Some of the code is using new C++11 features with arguably better APIs and practices. Some of the code is fairly old. If we were to write them from scratch today with enough time in our hands, end results may probably look quite different for some. For some, we are fairly happy with.

While choosing what to include in this package, we generally asked ourselves the following questions:

- Is the component potentially useful to others?
- Is the component high enough quality? Would potential users rather modify them
  to suit their needs or start from scratch?
- Is the component fairly independent?
- Do we have enough time / man power to clean it up and make it available?

If you have any comments or feedback, please let us know. We are open to receiving improvements and patches but we need to weigh general usefulness and complexity of integration.

-- Uygar Oztekin (oztekin@room77.com), and Room77 engineering team. Dec 2013.
