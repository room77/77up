// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

// This utility is useful to read expedia data (and similar) where the
// information may be split across multiple files. It allows record by record
// field by field inspection, extraction and translation. If this does not sound
// like what you may need you may want to use csvreader.h instead.

// See data/convert_expedia_data.cc for example usage.

#ifndef _PUBLIC_UTIL_FILE_FIELD_READER_H_
#define _PUBLIC_UTIL_FILE_FIELD_READER_H_

#include <sstream>

#include "base/common.h"
#include "util/file/monitor_file_access.h"
#include "util/serial/serializer.h"
#include "util/memory/unique.h"

class DelimitedFieldReader {
 public:
  DelimitedFieldReader(char delim = '|') : delim_(delim) { }
  virtual ~DelimitedFieldReader() {}

  // Parses a single line from stream and builds tag to field mappings.
  // Typically the stream is a file you just opened and the first line contains
  // the description of the fields.
  void ParseTags(istream& strm) {
    static const string bom = "\xef\xbb\xbf";
    string line;
    getline(strm, line);
    // Ignore BOM header.
    if (line.substr(0, 3) == bom) line = line.substr(3);
    // Ignore trailing CRs.
    if (!line.empty() && line[line.size()-1] == '\r') line.resize(line.size()-1);
    tags_ = Tokenize(line);
    tag_to_field_ = ReverseTagMap(tags_);
  }

  // Check if all the required fields are there
  bool HasTags(const vector<string>& required_tags) const {
    const unordered_set<string> tags_set(begin(tags_), end(tags_));
    vector<string> missing_tags;
    for (const string& tag : required_tags) {
      if (tags_set.find(tag) == tags_set.end()) missing_tags.push_back(tag);
    }
    if (missing_tags.size() > 0) {
      LOG(INFO) << "missing tags: " << serial::Serializer::ToJSON(missing_tags);
    }
    return missing_tags.size() == 0;
  }

  // return a vector of tags
  vector<string> GetTags() {
    return tags_;
  }

  // Parse the next line and make it ready for extraction.
  // Returns false on error.
  bool ParseLine(istream& strm) {
    // If we are in record file access mode, return immediately.
    if (gFlag_record_file_access) return false;

    while (!strm.fail()) {
      string line;
      getline(strm, line);
      // Ignore empty or commented-out lines
      if (!line.empty() && line[0] != '#') {
        // Ignore trailing CRs.
        if (line[line.size()-1] == '\r') {
          line.resize(line.size()-1);
        }
        data_ = Tokenize(line);
        break;
      }
    }
    return !strm.fail();
  }

  // Extract the field corresponding to tag number and set variable t with the
  // contents of the field. Variable's type is inferred automatically.
  // This should work for most built in data types.
  template<class T>
  bool Get(int n, T* t) const {
    if (data_.size() <= n) return false;
    stringstream strm(data_[n]);
    strm >> *t;
    return !strm.fail();
  }

  // This version maps the tag string to the appropriate tag number based on the
  // definition obtained through ParseTags(). This approach is more robust in
  // case field numbers change, new fields are inserted / added etc.
  template<class T>
  bool Get(const string& tag, T* t) const {
    map<string, int>::const_iterator it = tag_to_field_.find(tag);
    if (it == tag_to_field_.end()) return false;
    return Get(it->second, t);
  }

  template<class T, class Key, int Id>
  bool Get(const string& tag, ::util::unique<T, Key, Id>* t) const {
    T v;
    bool res = Get(tag, &v);
    if (res) *t = v;
    return res;
  }

  // Specialization for string.
  bool Get(int n, string* t) const {
    if (data_.size() <= n) return false;
    *t = data_[n];
    return true;
  }

  string DumpLast() const {
    stringstream ss;
    map<int, string> field_to_tag;
    for (auto it = tag_to_field_.begin(); it != tag_to_field_.end(); ++it)
      field_to_tag[it->second] = it->first;
    for (int i = 0; i < data_.size(); ++i) {
      ss << field_to_tag[i] << ": " << data_[i] << endl;
    }
    return ss.str();
  }

 protected:
  virtual vector<string> Tokenize(const string& line) {
    vector<string> content;
    stringstream strm(line);
    for (;;) {
      string token;
      getline(strm, token, delim_);
      if (strm.fail()) break;
      content.push_back(token);
    }
    return content;
  }

  // Build a reverse tag map.
  map<string, int> ReverseTagMap(const vector<string>& tags) {
    map<string, int> tag_to_field;
    for (int i = 0; i < tags.size(); ++i) {
      VLOG(4) << "Field : " << i << " Description : '" << tags[i] << "'";
      tag_to_field[tags[i]] = i;
    }
    return tag_to_field;
  }

  char delim_;
  vector<string> tags_;
  map<string, int> tag_to_field_;
  vector<string> data_;
};

// This class is used as a special case when individual fields may contain
// delimiter within the field. For example, the CSV format specifies that
// each field may have ',' within quotes.
class EscapedDelimitedFieldReader :  public DelimitedFieldReader {
 public:
  EscapedDelimitedFieldReader(char delim = '|', char esc = '"')
      : DelimitedFieldReader(delim), esc_(esc) { }
  virtual ~EscapedDelimitedFieldReader() {}

 protected:
  virtual vector<string> Tokenize(const string& line) {
    vector<string> content;
    // We start with looking for the next delimiter from the beginning.
    size_t pos = 0;
    while (pos < line.length()) {
      string token;
      // Check if the first content char is the esc_ char.
      if (line[pos] == esc_)
        pos = FindNextEscapedToken(line, pos + 1, &token);
      else
        pos = FindNextDelimitedToken(line, pos, &token);

      UnEscpaseString(&token);
      content.push_back(token);
    }
    return content;
  }

  size_t FindNextEscapedToken(const string& line, size_t pos, string* token) {
    size_t next = string::npos;
    while(pos < line.length()) {
      next = line.find(esc_, pos);
      if (next == string::npos) {
        LOG(INFO) << "Did not find matching escape char [" << esc_ << "] for "
               << "the field starting at position " << pos
               << " in line [" << line << "]";

        *token = "";
        return string::npos;
      }

      // We need to account for the case when the field contains escaped esc_
      // char. eg. a,"some ""quoted"" string", c. In this case the second
      // field needs to be parsed as [some "quoted" field].

      // Check if this is the end of the string. In that case we are done.
      if (next + 1 == line.size()) {
        token->append(line.substr(pos, next - pos));
        return string::npos;
      }

      // Check if this is the end of the field. In that case we are done.
      if (line[next + 1] == delim_) {
        token->append(line.substr(pos, next - pos));
        return next + 2; // Next valid field starts at next + 2.
      }

      if (line[next + 1] != esc_) {
        // Check if the file is using invalid escaped chars like \<esc_> instead
        // of <esc_><esc_>, i.e. escaping is like ["some \"quoted\" string"].
        if (next > 0 && line[next - 1] == '\\') {
          // Remove the '\', keep the esc_ and continue token.
          token->append(line.substr(pos, next - pos - 1));
          token->append(string(1, esc_));
          pos = next + 1;
          continue;
        } else {
          // The next char is neither the escape character or the delimeter.
          // This is again incorrect format.
          LOG(INFO) << "Did not find escape char [" << esc_ << "] or delimiter ["
                 << delim_ << "] for " << "the field expected to end at position "
                 << next << " in line [" << line << "]";
          *token = "";
          return string::npos;

        }
      }

      // This is the case where we have escaped esc_ char. Include one esc_ and
      // continue at next + 2.
      token->append(line.substr(pos, next - pos + 1));
      next += 2;
      pos = next;
    }
    return next;
  }

  size_t FindNextDelimitedToken(const string& line, size_t pos, string* token) {
    if (pos >= line.length()) return string::npos;

    size_t next = line.find(delim_, pos);
    // This will work even if next == npos.
    token->append(line.substr(pos, next - pos));

    return next == string::npos ? string::npos : next + 1;
  }

  void UnEscpaseString(string* str) {
    // Check if the file is using invalid escaped chars like \<esc_>
    // i.e. escaping is like [some \"quoted\" string] instead of
    // ["some ""quoted"" string"].
    const string escape(string(1, esc_));
    const string escaped_sub("\\" + escape);
    size_t pos = 0;
    while(pos < str->size()) {
      pos = str->find(escaped_sub, pos);
      if (pos == string::npos) break;

      str->replace(pos, 2, escape);
      ++pos;
    }
  }

  char esc_;
};

#endif  // _PUBLIC_UTIL_FILE_FIELD_READER_H_
