// Copyright 2011 B. Uygar Oztekin, Room77 Inc.
// Copied and modified from Uygar's personal library.
// Additions made by karepker@room77.com (Kar Epker)

#ifndef _PUBLIC_UTIL_COMPRESS_GZIP_H_
#define _PUBLIC_UTIL_COMPRESS_GZIP_H_

#include <functional>

#include "base/common.h"

namespace Compression {

// Compress the string using zlib library. Returns empty string upon error.
// Compression level can be between 0 to 9 (max). 0 means no compression.
string GzipCompress(const string& str, int comp_level = 9);

// Decompress the string using zlib library. Returns empty string upon error.
string GzipDecompress(const string& str);

// Calls the parser function for each line of a gzipped file.
// The parser can return a few options.
// +ve : The line was parsed successfully and we should continue to the next line.
//       NOTE: Currently we assume that each component is separated by a new line. If this changes
//             in future we can simply return the amount of line parsed and the rest can be added
//             to the subsequent component.
// 0 : There was error parsing the line. The file should not be processed any further.
// -ve : The line does not constitute the complete component. We need to parse more lines. This
//       allows us to have a single component in multiple lines.
// Returns number of components processed, -1 if file couldn't be opened.
int ProcessGzipLines(const string& path, const std::function<int (const string&)>& parser_func);

}  // namespace Compression

#endif  // _PUBLIC_UTIL_COMPRESS_GZIP_H_
