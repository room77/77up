#include "util/network/httputil.h"
#include "util/compress/gzip.h"

namespace HttpUtil {
  // parse HTTP response and obtain header and content
  bool ParseHttpResponse(const string& response,
                         int *ret_status_code,
                         tHeaderType *ret_header, string *ret_content) {
    int status;
    tHeaderType header;
    const char *header_begin = response.c_str();
    const char *after_header = ParseHttpHeader(header_begin, &status, &header);
    if (after_header == NULL)
      return false;  // error in HTTP header

    string content = response.substr(after_header - header_begin);
    if (IsChunkedResponse(header))
      content = ParseChunkedResponse(content);

    if (IsGzippedResponse(header))
      content = UncompressGzip(content);

    // return results to user
    if (ret_status_code)
      *ret_status_code = status;
    if (ret_header)
      *ret_header = header;
    if (ret_content)
      *ret_content = content;
    return true;
  }


  // parse HTTP header and return a pointer to the character after HTTP header
  // (return NULL if an error has occurred
  const char *ParseHttpHeader(const char *header_begin,
                              int *status_code, tHeaderType *header) {
    ASSERT(header_begin != NULL);
    if (status_code)
      *status_code = -1;
    if (header)
      header->clear();

    int line_num = 0;
    const char *line_begin = header_begin;
    while (*line_begin != '\0' &&
           *line_begin != '\r' && *line_begin != '\n') {
      // scan until the end of this line
      const char *line_end = line_begin;
      while (*line_end >= ' ')
        line_end++;

      line_num++;
      if (line_num == 1) {
        if (status_code != NULL) {
          // this is the first line.  Extract status code from this line.
          const char *s = strutil::SkipSpaces(line_begin);
          s = strutil::SkipNonSpaces(s);  // skip "HTTP/1.1"
          s = strutil::SkipSpaces(s);
          // now s points to the beginning of the status code
          *status_code = atoi(s);
        }
      }
      else {
        if (header != NULL) {
          // this is not the first line.  Parse as key:value format
          const char *colon = NULL;
          for (const char *s = line_begin; s < line_end; s++) {
            if (*s == ':') {
              colon = s;
              break;
            }
          }
          if (colon == NULL) {
            VLOG(2) << "Warning: no ':' found in header line " << line_num;
            return NULL;
          }
          // extract key and value from this line
          string key(line_begin, colon - line_begin);
          string value(colon + 1, line_end - (colon + 1));

          string header_key = strutil::GetTrimmedString(key);
          string header_value = strutil::GetTrimmedString(value);
          tHeaderType::iterator itr = header->find(header_key);
          if (itr == header->end()) {
            // this key has not appeared before
            (*header)[header_key] = header_value;
          }
          else {
            // this key has appeared before -- append after \n
            itr->second += '\n';
            itr->second.append(header_value);
          }
        }
      }

      if (*line_end == '\n')
        line_begin = line_end + 1;
      else if (*line_end == '\r' && *(line_end + 1) == '\n')
        line_begin = line_end + 2;
      else {
        // an error has occurred
        VLOG(2) << "Warning: unexpected character in HTTP header";
        return NULL;
      }
    }

    if (*line_begin == '\0') {
      // error: end of string encountered; there is no content
      VLOG(2) << "Warning: Invalid HTTP header (content not found)";
      return NULL;
    }
    else {
      // two consecutive newlines are found
      if (*line_begin == '\n')
        return (line_begin + 1);
      else if (*line_begin == '\r' && *(line_begin + 1) == '\n')
        return (line_begin + 2);
      else {
        // an error has occurred: \r is not followed by \n
        VLOG(2) << "Warning: Invalid HTTP header (\r not followed by \n)";
        return NULL;
      }
    }
  }

  // parse "chunked" response
  string ParseChunkedResponse(const string& raw) {
    VLOG(4) << "Parsing chunked response...";
    string result;
    const char *s = raw.c_str();
    const char *end = s + raw.size();
    char *next;
    int chunk_size = strtol(s, &next, 16);
    while (chunk_size > 0) {
      s = next;
      // skip spaces after chunk size
      while ((*s) == ' ' && s < end)
        s++;
      // skip newline
      if (*s == '\n')
        s++;
      else if (*s == '\r' && *(s + 1) == '\n')
        s += 2;
      else {
        VLOG(2) << "Error in chunked response: chunk size " << chunk_size
               << " is not followed by end-of-line";
        return "";
      }
      if (s + chunk_size >= end) {
        VLOG(2) << "Error in chunked response: "
               << "chunk size " << chunk_size << " too large";
        return "";
      }
      result += string(s, chunk_size);  // append this chunk
      s += chunk_size;
      // skip newline
      if (*s == '\n')
        s++;
      else if (*s == '\r' && *(s + 1) == '\n')
        s += 2;
      else {
        VLOG(2) << "Error in chunked response: chunk is not followed by CRLF";
        return "";
      }

      // get the next chunk
      chunk_size = strtol(s, &next, 16);
    }
    return result;
  }

  // uncompress gzipped content
  string UncompressGzip(const string& raw) {
    if (raw.empty()) return "";  // nothing to uncompress

    string result = Compression::GzipDecompress(raw);
    if (result.empty())
      VLOG(3) << "gzip uncompress failed. original size: " << raw.size();
    return result;
  }

  // compress using gzip
  string CompressGzip(const string& original, bool *success) {
    *success = false;
    string result = Compression::GzipCompress(original);
    if (!result.empty()) {
      *success = true;
      return result;
    }
    else {
      *success = false;
      LOG(INFO) << "gzip compress failed";
      return original;
    }
    return result;
  }
}
