#include "util/network/webserver.h"
#include "util/network/httputil.h"

FLAG_bool(log_full_post_requests, false, "dump full POST requests to log");

// check if all bytes have been received
// optional return values: request size, url, CGI arguments
bool WebServer::HttpRequestIsComplete(const string& r,
                                      unsigned int *request_size,
                                      string *url, string *cgi_arguments,
                                      string *input_cookie, string *referrer,
                                      bool *accept_gzip, bool *keep_alive,
                                      bool *is_post,
                                      unordered_map<string, string> *http_header) {
  // Consider a request complete if it contains two consecutive newlines,
  // except for a "POST" request, which requires additional content
  // after two consecutive newlines.
  const char *start = r.c_str();

  const char *s;

  const char *nn_newlines = strstr(start, "\n\n");
  const char *rnrn_newlines = strstr(start, "\r\n\r\n");

  if (nn_newlines == NULL) {
    s = rnrn_newlines;
  } else if (rnrn_newlines == NULL) {
    s = nn_newlines;
  } else {
    // if both newline types are found, choose the one that occurs first
    // to represent the end of the header
    s = (nn_newlines < rnrn_newlines) ? nn_newlines : rnrn_newlines;
  }

  if (s == NULL)
    return false;

  const char *after_two_newlines = (((*s) == '\n') ? (s + 2) : (s + 4));
  unsigned int header_len = after_two_newlines - start;

  bool request_is_post = (strncasecmp(start, "POST", 4) == 0);
  bool request_is_complete = false;
  int content_length = 1;

  unordered_map<string, string> http_header_local;
  // in case caller didn't supply http_header parameter
  if (http_header == NULL)
    http_header = &http_header_local;

  vector<string> lines;
  strutil::BreakUpString(r.substr(0, header_len), '\n', &lines);
  for (int i = 1; i < lines.size(); i++) {
    const char *s = lines[i].c_str();
    const char *sep = strchr(s, ':');  // find the first ':'
    if (sep != NULL) {
      string key = strutil::GetTrimmedString(string(s, sep - s));
      string value = strutil::GetTrimmedString(string(sep + 1));
      (*http_header)[key] = value;
    }
  }

  if (input_cookie != NULL) {
    // get Cookie from header
    unordered_map<string, string>::const_iterator itr = http_header->find("Cookie");
    *input_cookie = (itr == http_header->end() ? "" : itr->second);
  }
  if (referrer != NULL) {
    // get Referer from header
    unordered_map<string, string>::const_iterator itr = http_header->find("Referer");
    *referrer = (itr == http_header->end() ? "" : itr->second);
  }
  if (accept_gzip != NULL) {
    // check if client accepts gzip encoding
    unordered_map<string, string>::const_iterator itr =
      http_header->find("Accept-Encoding");
    *accept_gzip = (itr != http_header->end() &&
                    strstr(itr->second.c_str(), "gzip") != NULL);
  }

  if (keep_alive != NULL) {
    // check if client requests keep-alive connection
    unordered_map<string, string>::const_iterator itr =
      http_header->find("Connection");
    *keep_alive = (itr != http_header->end() &&
                   strstr(itr->second.c_str(), "keep-alive") != NULL);
  }

  /*
  if (input_cookie != NULL) {
    // retrieve HTTP cookie, if available
    input_cookie->clear();
    const char *ck = strstr(start, "Cookie: ");
    if (ck != NULL && ck < after_two_newlines) {
      const char *cookie_begin = ck + 8;
      const char *cookie_end = cookie_begin;
      while ((*cookie_end) >= ' ')
        cookie_end++;
      *input_cookie = string(cookie_begin, cookie_end - cookie_begin);
    }
  }
  */

  if (request_is_post) {
    // a post request requires additional content
    const char *cl = strstr(start, "Content-Length: ");
    if (cl != NULL && cl < after_two_newlines)
      content_length = atoi(cl + 16);
    else
      content_length = 1;

    unsigned int total_size = header_len + content_length;
    if (request_size != NULL)
      *request_size = total_size;

    request_is_complete = (r.size() >= total_size);
  }
  else {
    if (request_size != NULL)
      *request_size = header_len;
    request_is_complete = true;
  }

  if (request_is_complete) {
    // fill in other optional return values
    if (is_post)
      *is_post = request_is_post;

    const char *s = start;
    while ((*s) > ' ') s++;  // scan until the first space character
    if ((*s) != '\0') s++;
    // now s points to the beginning of URL
    const char *url_begin = s;

    while ((*s) > ' ' && (*s) != '?') s++;  // scan until the first space or ?

    // now s points to the character just after URL
    if (url)
      *url = string(url_begin, s - url_begin);

    if (request_is_post) {
      // POST command
      // cgi arguments can be found just after two consecutive newlines
      if (cgi_arguments)
        *cgi_arguments = string(after_two_newlines, content_length);
    }
    else {
      // GET command
      // cgi arguments can be found after '?' in URL
      if ((*s) == '?') {
        s++;
        const char *cgi_begin = s;
        while ((*s) > ' ') s++;
        // now s points to the character just after CGI arguments
        if (cgi_arguments)
          *cgi_arguments = string(cgi_begin, s - cgi_begin);
      }
      else {
        // no GET arguments
        if (cgi_arguments)
          *cgi_arguments = "";
      }
    }
  }

  return request_is_complete;
}


// process a request
string WebServer::ProcessRequest(const string& request,
                                 bool *keep_alive,
                                 const tConnectionInfo *connection) {
  unsigned int request_len;
  string url, cgi_arguments, input_cookie, referrer;
  unordered_map<string, string> http_header;
  bool accept_gzip, is_post;

  if (HttpRequestIsComplete(request, &request_len,
                            &url, &cgi_arguments, &input_cookie, &referrer,
                            &accept_gzip, keep_alive, &is_post, &http_header)) {
    VLOG(4) << "processrequest:" << request;
    LogCaller(connection->caller_id,
              HttpRequestSummary(url, cgi_arguments, is_post));

    string result_content, content_type;
    int max_cache_seconds = 0;
    vector<string> empty_cookies;

    int response_code = gHttpResponse_ServerError;

    if (!(url.empty()))
      response_code = ProcessHttpRequest(url, is_post, cgi_arguments,
                                         input_cookie, referrer,
                                         &result_content, &content_type,
                                         &max_cache_seconds);
    return ConstructHttpResponse(response_code, result_content, content_type,
                                 accept_gzip, empty_cookies, max_cache_seconds);
  }
  else {
    // request is incomplete (this shouldn't happen)
    return "";
  }
}

// assemble http response from components
// (note: in case response_code is not gHttpResponse_OK, all other arguments
//        are ignored)
string WebServer::ConstructHttpResponse(int response_code,
                                        const string& result_content,
                                        const string& content_type,
                                        bool client_accepts_gzip,
                                        const vector<string>& full_cookie_specs,
                                        int max_cache_seconds) {
  stringstream header;
  string content;

  switch (response_code) {
  case gHttpResponse_OK: {
    content = result_content;
    header << "HTTP/1.1 200 Document follows\n"
           << "Content-type: " << content_type << "; charset=utf-8\n"
           << "Cache-Control: private, max-age=" << max_cache_seconds << "\n";
    break;
  }
  case gHttpResponse_NotFound: {
    content =
      "<html><head><title>404 Not Found</title></head>\n"
      "<body><h1>Not Found</h1>\n"
      "The requested URL was not found on this server.<p><hr></body>\n"
      "</html>\n";

    header << "HTTP/1.1 404 Document not found\n"
           << "Content-type: text/html\n";
    break;
  }
  case gHttpResponse_ServerBusy: {
    content =
      "<html><head><title>Server Busy</title></head>\n"
      "<body><h1>Server Busy</h1>\n"
      "The server is busy at this moment.  Please try again later.<p><hr>"
      "</body>\n</html>\n";

    header << "HTTP/1.1 503 Server Busy\n"
           << "Content-type: text/html\n";
    break;
  }
  default: {
    // default: internal server error
    content =
      "<html><head><title>Internal Server Error</title></head>\n"
      "<body><h1>Internal Server Error</h1>\n"
      "The server has encountered an error.  Please try again later.<p><hr>"
      "</body>\n</html>\n";

    header << "HTTP/1.1 500 Server Error\n"
           << "Content-type: text/html\n";
    break;
  }
  }

  if (!full_cookie_specs.empty()) {
    for(auto& full_cookie_spec : full_cookie_specs) {
      header << "Set-Cookie: " << full_cookie_spec << "\n";
    }
  }

  // try to use gzip if client accepts it, and content is large enough
  if (client_accepts_gzip && content.size() > 1024) {
    bool success;
    content = HttpUtil::CompressGzip(content, &success);
    // if unsuccessful, content is not changed
    if (success)
      header << "Content-Encoding: gzip\n";
  }

  header << "Content-length: " << content.size() << "\n";

  VLOG(5) << "Response header:\n" << header.str();
  return header.str() + "\n" + content;
}


// return a summary string for logging purposes
string WebServer::HttpRequestSummary(const string& url,
                                     const string& cgi_arguments,
                                     bool is_post) {
  stringstream ss;
  if (is_post) {
    ss << "POST " << url;
    if (!cgi_arguments.empty()) {
      string unescaped = strutil::UnescapeString_CGI(cgi_arguments);
      if (gFlag_log_full_post_requests)
        ss << " ... " << unescaped;
      else {
        ss << " ... " << unescaped.substr(0, 300);
        if (unescaped.size() > 300)
          ss << " ... (total " << unescaped.size() << " bytes)";
      }
    }
  }
  else {
    ss << "GET " << url;
    if (!cgi_arguments.empty())
      ss << "?" << cgi_arguments;
  }
  return ss.str();
}

// dummy implementation of ProcessHttpRequest
int WebServer::ProcessHttpRequest(const string& url,
                                  bool is_post,
                                  const string& cgi_arguments,
                                  const string& input_cookie,
                                  const string& referrer,
                                  // return values below
                                  string *result_content,
                                  string *content_type,
                                  int *max_cache_seconds) {
  ASSERT(result_content != NULL);
  ASSERT(content_type != NULL);
  ASSERT(max_cache_seconds != NULL);

  *result_content =
    "<html><head><title>Test</title></head>\n"
    "<body><h2>This is a test.</h2></body></html>\n";
  *content_type = "text/html";
  *max_cache_seconds = 5;

  return WebServer::gHttpResponse_OK;
}


// guess content type from file name suffix
string WebServer::GuessContentType(const string& filename) {
  int pos = filename.rfind('.');
  if (pos == string::npos)
    return "text/plain";
  else {
    string suffix = filename.substr(pos + 1);
    for (int i = 0; i < suffix.size(); i++)
      suffix[i] = tolower(suffix[i]);
    if (suffix == "jpg" || suffix == "jpeg")
      return "image/jpeg";
    else if (suffix == "gif")
      return "image/gif";
    else if (suffix == "png")
      return "image/png";
    else if (suffix == "tif" || suffix == "tiff")
      return "image/tiff";
    else if (suffix == "html" || suffix == "htm")
      return "text/html";
    else if (suffix == "xml")
      return "text/xml";
    else if (suffix == "js")
      return "application/x-javascript";
    else if (suffix == "css")
      return "text/css";
    else if (suffix == "ico")
      return "image/vnd.microsoft.icon";
    else
      return "text/plain";
  }
}

