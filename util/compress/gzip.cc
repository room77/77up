// Copyright 2011 B. Uygar Oztekin, Room77 Inc.
// Copied and modified from Uygar's personal library.
// Additions made by karepker@room77.com (Kar Epker)

// @flags: -lz

#include "gzip.h"

#include <cstring>
#include <zlib.h>
#include "base/defs.h"


FLAG_int(char_buffer_length, 5000,
  "The initial size of the buffer into which to read the line");

namespace Compression {

// Compress the string using zlib library. Returns empty string upon error.
string GzipCompress(const string& str, int comp_level) {
  if (str.empty()) return str;
  // Allocate the same amount of buffer as a first approximation.
  string output(str.size(), ' ');

  // Create the stream and set the input.
  z_stream zs;
  memset(&zs, 0, sizeof(zs));
  // Params: stream, compression level, method, windowbits, mem level, strategy.
  // +16 in windowbits means gzip. Mem level should be 1 to 9. 1 means low
  // memory use / slow. 9 means high memory use, fast. 8 was recommended as a
  // good balance. See /usr/include/zlib.h for more options.
  ASSERT(deflateInit2(&zs, comp_level, Z_DEFLATED, 15 + 16, 8,
                      Z_DEFAULT_STRATEGY) == Z_OK);

  zs.next_in = (Bytef*)str.data();
  zs.avail_in = str.size();

  // Compress in blocks without using a temporary buffer.
  // Note that resulting string can be bigger than original.
  for (;;) {
    zs.next_out = (Bytef*)(output.data() + zs.total_out);
    zs.avail_out = output.size() - zs.total_out;

    int status = deflate(&zs, Z_FINISH);
    if (status == Z_OK) output.resize(output.size() * 2);
    else if (status == Z_STREAM_END) {
      output.resize(zs.total_out);
      break;
    }
    else {
      VLOG(2) << "Error compressing string.";
      output.resize(0);
      break;
    }
  }
  deflateEnd(&zs);
  return output;
}

// Decompress the string using zlib library. Returns empty string upon error.
string GzipDecompress(const string& str) {
  if (str.empty()) return str;
  // Expect a 1.5x increase in size as a first approximation.
  string output(str.size() * 1.5, ' ');

  // Create the stream and set the input.
  z_stream zs;
  memset(&zs, 0, sizeof(zs));
  // +32 means gzip deflate. See /usr/include/zlib.h for more information.
  ASSERT(inflateInit2(&zs, 15 + 32) == Z_OK);
  zs.next_in = (Bytef*)str.data();
  zs.avail_in = str.size();

  // Decompress in blocks.
  for (;;) {
    zs.next_out = (Bytef*)(output.data() + zs.total_out);
    zs.avail_out = output.size() - zs.total_out;

    int status = inflate(&zs, 0);
    if (status == Z_OK) output.resize(output.size() * 2);
    else if (status == Z_STREAM_END) {
      output.resize(zs.total_out);
      break;
    }
    else {
      VLOG(2) << "Error decompressing string.";
      output.resize(0);
      break;
    }
  }
  inflateEnd(&zs);
  return output;
}

int ProcessGzipLines(const string& path, const std::function<int (const string&)>& parser_func) {
  // open the file
  gzFile log_file = gzopen(path.c_str(), "rb");
  // make sure the file can be opened
  if (log_file == nullptr) return -1;
  string line;
  int lines_processed = 0;
  char buffer[gFlag_char_buffer_length];
  // make sure the last character is a newline so we got the whole line
  // note: gzgets will read at most buffer_length - 1 characters
  while (gzgets(log_file, buffer, gFlag_char_buffer_length)) {
    line.append(buffer);
    if (line.rbegin()[0] == '\n') {
      int status = parser_func(line);
      if (status > 0) {  // Optimize for most common case.
        line.clear();
        ++lines_processed;
        continue;
      }

      // Check if we need more lines.  (Next most frequent case.)
      if (status < 0) continue;

      // The parser had a failure. Need to break.
      break;
    }
  }
  gzclose(log_file);
  return lines_processed;
}

}  // namespace Compression
