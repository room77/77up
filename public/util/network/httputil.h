#ifndef _PUBLIC_UTIL_NETWORK_HTTPUTIL_H_
#define _PUBLIC_UTIL_NETWORK_HTTPUTIL_H_

//
// HTTP-related utilities
//

#include "base/common.h"
#include "util/hash/hash_util.h"
#include "util/string/strutil.h"

namespace HttpUtil {

// case-insensitive
typedef unordered_map<string, string, ::hash::string_casefold_hash,
    ::hash::string_casefold_eq> tHeaderType;

// parse HTTP response and obtain header and content
bool ParseHttpResponse(const string& response,
                       int *ret_status_code,
                       tHeaderType *ret_header, string *ret_content);

// parse HTTP header and return a pointer to the character after HTTP header
const char *ParseHttpHeader(const char *header_begin,
                            int *status_code, tHeaderType *header);

// check if the header indicates "chunked" response
inline bool IsChunkedResponse(const tHeaderType& header) {
  tHeaderType::const_iterator i = header.find("Transfer-Encoding");
  if (i == header.end())
    return false;
  else
    return (strcasestr(i->second.c_str(), "chunked") != NULL);
}

// check if the header indicates gzip encoding
inline bool IsGzippedResponse(const tHeaderType& header) {
  tHeaderType::const_iterator i = header.find("Content-Encoding");
  if (i == header.end())
    return false;
  else
    return (strcasestr(i->second.c_str(), "gzip") != NULL);
}

// parse "chunked" response
string ParseChunkedResponse(const string& raw);

// uncompress gzipped content
string UncompressGzip(const string& raw);

// compress using gzip
// (if successful, return compressed content;
//  if failed, return original content)
string CompressGzip(const string& original, bool *success);

}  // namespace HttpUtil

#endif  // _PUBLIC_UTIL_NETWORK_HTTPUTIL_H_
